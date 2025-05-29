#ifndef DENSE_MAP_SIMD_HPP
#define DENSE_MAP_SIMD_HPP

#define HXSL_AVX2 1
//#undef HXSL_AVX2

#include "pch/std.hpp"
#include "config.h"
#include "memory.hpp"

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
// GCC/Clang: use __builtin_ctz if available
static inline unsigned int count_trailing_zeros(uint32_t x) {
	return __builtin_ctz(x);
}
#elif defined(_MSC_VER)
#include <intrin.h>
static inline unsigned int count_trailing_zeros(uint32_t x) {
	unsigned long index;
	_BitScanForward(&index, x);
	return index;
}
#else
// Portable fallback if no compiler intrinsic available
static inline unsigned int count_trailing_zeros(uint32_t x)
{
	unsigned int count = 0;
	while ((x & 1) == 0)
	{
		x >>= 1;
		count++;
	}
	return count;
}
#endif

template <typename TKey, typename TValue, typename Hash = std::hash<TKey>, typename KeyEqual = std::equal_to<TKey>, typename Allocator = simd_allocator<uint8_t>>
class dense_map_simd
{
public:
	using pair = std::pair<const TKey, TValue>;

private:

	enum class EntryFlags : char
	{
		Empty = 0,
		Tombstone = 1,
		Filled = 2,
	};

private:
	float load_factor_m = 0.75f;
	uint8_t* base = nullptr;
	size_t capacity_m = 0;
	size_t size_m = 0;
	Allocator allocator;
	Hash hasher;
	KeyEqual key_equal;

	struct BucketColumns
	{
		size_t* hashes;
		EntryFlags* flags;
		pair* pairs;

		BucketColumns operator+(size_t offset) const
		{
			return { hashes + offset, flags + offset, pairs + offset };
		}

		BucketColumns operator-(size_t offset) const
		{
			return { hashes - offset, flags - offset, pairs - offset };
		}

		BucketColumns& operator+=(size_t offset)
		{
			hashes += offset;
			flags += offset;
			pairs += offset;
			return *this;
		}

		BucketColumns& operator-=(size_t offset)
		{
			hashes -= offset;
			flags -= offset;
			pairs -= offset;
			return *this;
		}

		BucketColumns& operator++()
		{
			++hashes;
			++flags;
			++pairs;
			return *this;
		}

		BucketColumns operator++(int)
		{
			BucketColumns temp = *this;
			++(*this);
			return temp;
		}

		BucketColumns& operator--()
		{
			--hashes;
			--flags;
			--pairs;
			return *this;
		}

		BucketColumns operator--(int)
		{
			BucketColumns temp = *this;
			--(*this);
			return temp;
		}
	};

	static constexpr size_t VECTOR_WIDTH = 32;
	static constexpr size_t HASH_STRIDE = VECTOR_WIDTH / sizeof(size_t);

	BucketColumns GetColumns(uint8_t* base, size_t capacity)
	{
		const size_t paddedCapacity = capacity + SIMD::alignment;
		auto hashes = reinterpret_cast<size_t*>(base);

		size_t offset0 = paddedCapacity * sizeof(size_t);
		auto flags = reinterpret_cast<EntryFlags*>(base + offset0);

		size_t offset1 = offset0 + paddedCapacity * sizeof(EntryFlags);
		size_t offset2 = AlignUp(offset1, alignof(pair));
		auto pairs = reinterpret_cast<pair*>(base + offset2);

		return { hashes, flags, pairs };
	}

	inline void ClearColumns(uint8_t* base, size_t size, size_t capacity)
	{
		const auto cols = GetColumns(base, capacity);

		std::memset(base, 0, size);
		std::uninitialized_value_construct(cols.pairs, cols.pairs + capacity);
	}

	inline size_t ComputeSize(size_t capacity)
	{
		size_t baseSize = (sizeof(size_t) + sizeof(EntryFlags)) * (capacity + SIMD::alignment);
		size_t aligned = AlignUp(baseSize, alignof(pair));
		size_t pairsSize = sizeof(pair) * (capacity + HASH_STRIDE);
		return aligned + pairsSize;
	}

	BucketColumns find_entry(uint8_t* base, size_t capacity, const TKey& key, size_t hash)
	{
		const auto cols = GetColumns(base, capacity);
		size_t* hashes = cols.hashes;
		EntryFlags* flags = cols.flags;
		pair* pairs = cols.pairs;

		size_t tombstone = capacity;
		size_t index = hash & (capacity - 1);
		const size_t start = index;

#if HXSL_AVX2
		const __m256i target_hash = _mm256_set1_epi64x(hash);
		const __m256i empty_flag = _mm256_set1_epi8(static_cast<char>(EntryFlags::Empty));
		const __m256i tombstone_flag = _mm256_set1_epi8(static_cast<char>(EntryFlags::Tombstone));
		const __m256i shuffle_shift = _mm256_set1_epi8(HASH_STRIDE);
		alignas(32) static const uint8_t shuffle_bytes_0[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };

		size_t first_empty = capacity;
		while (true)
		{
			__m256i flag_vec = _mm256_load_si256((__m256i*) & flags[index]);
			__m256i cmp_empty = _mm256_cmpeq_epi8(flag_vec, empty_flag);
			if (!_mm256_testz_si256(cmp_empty, cmp_empty))
			{
				int empty_mask = _mm256_movemask_epi8(cmp_empty);
				int first = count_trailing_zeros(empty_mask);
				if (first == 0)
				{
					return cols + (tombstone != capacity ? tombstone : index);
				}
				first_empty = index + first;
			}

			__m256i cmp_tomb = _mm256_cmpeq_epi8(flag_vec, tombstone_flag);
			if (tombstone == capacity && !_mm256_testz_si256(cmp_tomb, cmp_tomb))
			{
				int tomb_mask = _mm256_movemask_epi8(cmp_tomb);
				int first = count_trailing_zeros(tomb_mask);
				tombstone = index + first;
			}

			__m256i empty_or_tomb = _mm256_or_si256(cmp_empty, cmp_tomb);
			__m256i occupied_mask = _mm256_andnot_si256(empty_or_tomb, _mm256_set1_epi8((char)0xFF));

			_mm_prefetch((const char*)&flags[(index + VECTOR_WIDTH) & (capacity - 1)], _MM_HINT_T0);

			if (!_mm256_testz_si256(occupied_mask, occupied_mask))
			{
				__m256i shuffle_mask = _mm256_load_si256((__m256i*)shuffle_bytes_0);

				for (size_t i = 0; i < VECTOR_WIDTH; i += HASH_STRIDE)
				{
					const size_t idx = index + i;
					__m256i occ_lane = _mm256_shuffle_epi8(occupied_mask, shuffle_mask);
					__m256i hash_vec = _mm256_load_si256((__m256i*) & hashes[idx]);
					__m256i cmp_hash = _mm256_cmpeq_epi64(hash_vec, target_hash);
					cmp_hash = _mm256_and_epi64(cmp_hash, occ_lane);

					if (_mm256_testz_si256(cmp_hash, cmp_hash)) break;
					int hash_mask = _mm256_movemask_pd(_mm256_castsi256_pd(cmp_hash));
					if (hash_mask & 0x1 && key_equal(pairs[idx].first, key)) { return cols + idx; }
					if (hash_mask & 0x2 && key_equal(pairs[idx + 1].first, key)) { return cols + idx + 1; }
					if (hash_mask & 0x4 && key_equal(pairs[idx + 2].first, key)) { return cols + idx + 2; }
					if (hash_mask & 0x8 && key_equal(pairs[idx + 3].first, key)) { return cols + idx + 3; }

					shuffle_mask = _mm256_add_epi8(shuffle_mask, shuffle_shift);
				}

				if (first_empty != capacity)
				{
					return cols + (tombstone != capacity ? tombstone : first_empty);
				}
			}

			index = (index + VECTOR_WIDTH) & (capacity - 1);
			if (index == start)
			{
				break;
			}
		}
#else

		do
		{
			auto& flag = flags[index];
			if (flag == EntryFlags::Empty)
			{
				return cols + (tombstone != capacity ? tombstone : index);
			}
			else if (flag == EntryFlags::Tombstone)
			{
				if (tombstone == capacity)
				{
					tombstone = index;
				}
			}
			else if (hashes[index] == hash && key_equal(pairs[index].first, key))
			{
				return cols + index;
			}

			index = (index + 1) & (capacity - 1);
		} while (index != start);
#endif
		throw std::runtime_error("Infinite loop detected in find_entry.");
	}

	void resize(size_t newCapacity)
	{
		if (newCapacity < size_m)
		{
			throw std::runtime_error("Cannot reduce capacity below current size");
		}

		size_t size = ComputeSize(newCapacity);
		uint8_t* newBase = allocator.allocate(size, SIMD::alignment);
		try
		{
			ClearColumns(newBase, size, newCapacity);
			if (base != nullptr)
			{
				auto colsSrc = GetColumns(base, capacity_m);

				for (size_t i = 0; i < capacity_m; ++i)
				{
					if (colsSrc.flags[i] != EntryFlags::Filled) continue;
					size_t& hash = colsSrc.hashes[i];
					pair& kv = colsSrc.pairs[i];
					auto colsDst = find_entry(newBase, newCapacity, kv.first, hash);
					*colsDst.hashes = hash;
					*colsDst.flags = EntryFlags::Filled;
					new (colsDst.pairs) pair(std::move(kv));
					kv.~pair();
				}
				allocator.deallocate(base, ComputeSize(capacity_m));
			}

			base = newBase;
			capacity_m = newCapacity;
		}
		catch (...)
		{
			allocator.deallocate(newBase, size);
			throw;
		}
	}

	void dealloc_entries()
	{
		auto cols = GetColumns(base, capacity_m);
		for (size_t i = 0; i < capacity_m; ++i)
		{
			auto& flag = cols.flags[i];
			if (flag != EntryFlags::Filled) continue;
			pair& kv = cols.pairs[i];
			kv.~pair();
			flag = EntryFlags::Empty;
		}
	}

public:
	dense_map_simd()
	{
		allocator = {};
	}

	dense_map_simd(const Allocator& alloc) : allocator(alloc)
	{
	}

	dense_map_simd(const Hash& hasher, const KeyEqual& equals, const Allocator& alloc) : hasher(hasher), key_equal(equals), allocator(alloc)
	{
	}

	~dense_map_simd()
	{
		if (base)
		{
			try
			{
				dealloc_entries();
			}
			catch (...)
			{
			}
			allocator.deallocate(base, ComputeSize(capacity_m));
			base = nullptr;
			capacity_m = 0;
			size_m = 0;
		}
	}

	size_t next_power_of_two(size_t n)
	{
		if (n == 0) return 1;
		n--;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		if constexpr (sizeof(size_t) == 8)
			n |= n >> 32;
		return n + 1;
	}

	void reserve(size_t capacity)
	{
		if (capacity > capacity_m * load_factor_m)
		{
			size_t newCapacity = capacity_m * 2;
			if (newCapacity < capacity)
			{
				newCapacity = next_power_of_two(capacity);
			}
			resize(std::max(newCapacity, static_cast<size_t>(32)));
		}
	}

	void clear()
	{
		dealloc_entries();
		size_m = 0;
	}

	void shrink_to_fit()
	{
		resize(size_m);
	}

	size_t size() const { return size_m; }
	size_t capacity() const { return capacity_m; }
	bool empty() const { return size_m == 0; }
	float load_factor() const { return load_factor_m; }
	void load_factor(float value) { load_factor_m = value; }

	Allocator get_allocator() const { return allocator; }
	Hash hash_function() const { return hasher; }
	KeyEqual key_eq() const { return key_equal; }

	template <typename P>
	void insert(P&& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		auto cols = find_entry(base, capacity_m, key, hash);

		if (*cols.flags == EntryFlags::Filled)
		{
			return;
		}

		*cols.flags = EntryFlags::Filled;
		*cols.hashes = hash;
		new (cols.pairs) pair(std::forward<P>(kv));

		++size_m;
	}

	void insert(pair&& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		auto cols = find_entry(base, capacity_m, key, hash);

		if (*cols.flags == EntryFlags::Filled)
		{
			return;
		}

		*cols.flags = EntryFlags::Filled;
		*cols.hashes = hash;
		new (cols.pairs) pair(std::move(kv));

		++size_m;
	}

	void insert(const pair& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		auto cols = find_entry(base, capacity_m, key, hash);

		if (*cols.flags == EntryFlags::Filled)
		{
			return;
		}

		*cols.flags = EntryFlags::Filled;
		*cols.hashes = hash;
		new (cols.pairs) pair(kv);

		++size_m;
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		pair kv(std::forward<Args>(args)...);
		insert(std::move(kv));
	}
};

#endif