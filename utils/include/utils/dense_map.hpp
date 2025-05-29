#ifndef DENSE_MAP_HPP
#define DENSE_MAP_HPP

#include "pch/std.hpp"

template <typename TKey, typename TValue, typename Hash = std::hash<TKey>, typename KeyEqual = std::equal_to<TKey>, typename Allocator = std::allocator<std::pair<const TKey, TValue>>>
class dense_map
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

	struct Entry
	{
		EntryFlags flags;
		size_t hash;
		pair pair;

		Entry() = default;

		template<typename P>
		Entry(EntryFlags flags, size_t hash, P&& pair) : flags(flags), hash(hash), pair(std::forward<P>(pair)) {}

		bool isEmpty() const { return flags == EntryFlags::Empty; }
		bool isTombstone() const { return flags == EntryFlags::Tombstone; }
		bool isFilled() const { return flags == EntryFlags::Filled; }
	};

public:
	using entry_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Entry>;
private:
	float load_factor_m = 0.75f;
	Entry* buckets = nullptr;
	size_t capacity_m = 0;
	size_t size_m = 0;
	entry_allocator allocator;
	Hash hasher;
	KeyEqual key_equal;

	Entry* find_entry(Entry* entries, size_t capacity, const TKey& key, size_t hash)
	{
		Entry* tombstone = nullptr;
		size_t index = hash & (capacity - 1);
		const size_t start = index;
		do
		{
			Entry* entry = &entries[index];
			if (!entry->isFilled())
			{
				if (entry->isEmpty())
				{
					return tombstone != nullptr ? tombstone : entry;
				}
				else
				{
					if (tombstone == nullptr)
					{
						tombstone = entry;
					}
				}
			}
			else if (entry->hash == hash && key_equal(entry->pair.first, key))
			{
				return entry;
			}

			index = (index + 1) & (capacity - 1);
		} while (index != start);

		// this should never happen only if someone forgot to call reserve.
		throw std::runtime_error("Infinite loop detected in find_entry.");
	}

	const Entry* cfind_entry(const Entry* entries, size_t capacity, const TKey& key, size_t hash) const
	{
		const Entry* tombstone = nullptr;
		size_t index = hash & (capacity - 1);
		const size_t start = index;
		do
		{
			const Entry* entry = &entries[index];
			if (!entry->isFilled())
			{
				if (entry->isEmpty())
				{
					return tombstone != nullptr ? tombstone : entry;
				}
				else
				{
					if (tombstone == nullptr)
					{
						tombstone = entry;
					}
				}
			}
			else if (entry->hash == hash && key_equal(entry->pair.first, key))
			{
				return entry;
			}

			index = (index + 1) & (capacity - 1);
		} while (index != start);

		// this should never happen only if someone forgot to call reserve.
		throw std::runtime_error("Infinite loop detected in find_entry.");
	}

	void resize(size_t newCapacity)
	{
		if (newCapacity < size_m)
		{
			throw std::runtime_error("Cannot reduce capacity below current size");
		}

		Entry* newBuckets = allocator.allocate(newCapacity);
		try
		{
			std::uninitialized_value_construct(newBuckets, newBuckets + newCapacity);

			if (buckets != nullptr)
			{
				for (size_t i = 0; i < capacity_m; ++i)
				{
					Entry& entry = buckets[i];
					if (!entry.isFilled()) continue;
					Entry& dest = *find_entry(newBuckets, newCapacity, entry.pair.first, entry.hash);
					new (&dest) Entry(std::move(entry));
					entry.~Entry();
				}
				allocator.deallocate(buckets, capacity_m);
			}

			buckets = newBuckets;
			capacity_m = newCapacity;
		}
		catch (...)
		{
			allocator.deallocate(newBuckets, newCapacity);
			throw;
		}
	}

	void dealloc_entries()
	{
		for (size_t i = 0; i < capacity_m; ++i)
		{
			Entry& entry = buckets[i];
			if (!entry.isFilled()) continue;
			entry.~Entry();
			entry.flags = EntryFlags::Empty;
		}
	}

public:
	dense_map()
	{
		allocator = {};
	}

	dense_map(const entry_allocator& alloc) : allocator(alloc)
	{
	}

	dense_map(const Hash& hasher, const KeyEqual& equals, const entry_allocator& alloc) : hasher(hasher), key_equal(equals), allocator(alloc)
	{
	}

	dense_map(const dense_map& other) : capacity_m(0), size_m(0), buckets(nullptr), allocator(other.allocator), hasher(other.hasher), key_equal(other.key_equal)
	{
		if (other.size_m > 0)
		{
			resize(other.capacity_m);
			for (const auto& kv : other)
			{
				insert(kv);
			}
		}
	}

	dense_map(dense_map&& other) noexcept
		: buckets(other.buckets), capacity_m(other.capacity_m), size_m(other.size_m),
		allocator(std::move(other.allocator)), hasher(std::move(other.hasher)), key_equal(std::move(other.key_equal))
	{
		other.buckets = nullptr;
		other.capacity_m = 0;
		other.size_m = 0;
	}

	~dense_map()
	{
		if (buckets)
		{
			try
			{
				dealloc_entries();
			}
			catch (...)
			{
			}
			allocator.deallocate(buckets, capacity_m);
			buckets = nullptr;
			capacity_m = 0;
			size_m = 0;
		}
	}

	class iterator
	{
		friend class dense_map;
		Entry* ptr;
		Entry* end_ptr;

		void skip_to_filled()
		{
			while (ptr != end_ptr && !ptr->isFilled())
			{
				++ptr;
			}
		}

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = pair;
		using difference_type = std::ptrdiff_t;
		using pointer = pair*;
		using reference = pair&;

		iterator(Entry* p, Entry* end) : ptr(p), end_ptr(end)
		{
			skip_to_filled();
		}

		reference operator*() const { return ptr->pair; }
		pointer operator->() const { return &ptr->pair; }

		iterator& operator++()
		{
			++ptr;
			skip_to_filled();
			return *this;
		}

		iterator operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const iterator& other) const { return ptr == other.ptr; }
		bool operator!=(const iterator& other) const { return ptr != other.ptr; }
	};

	iterator begin() { return iterator(buckets, buckets ? buckets + capacity_m : nullptr); }
	iterator end() { return iterator(buckets ? buckets + capacity_m : nullptr, buckets ? buckets + capacity_m : nullptr); }

	class const_iterator
	{
		const Entry* ptr;
		const Entry* end_ptr;

		void skip_to_filled() {
			while (ptr != end_ptr && !ptr->isFilled())
			{
				++ptr;
			}
		}

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = pair;
		using difference_type = std::ptrdiff_t;
		using pointer = const pair*;
		using reference = const pair&;

		const_iterator(const Entry* p, const Entry* end) : ptr(p), end_ptr(end)
		{
			skip_to_filled();
		}

		const_iterator(const iterator& it) : ptr(it.ptr), end_ptr(it.end_ptr) {}

		reference operator*() const { return ptr->pair; }
		pointer operator->() const { return &ptr->pair; }

		const_iterator& operator++()
		{
			++ptr;
			skip_to_filled();
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
		bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
	};

	const_iterator begin() const { return const_iterator(buckets, buckets ? buckets + capacity_m : nullptr); }
	const_iterator end() const { return const_iterator(buckets ? buckets + capacity_m : nullptr, buckets ? buckets + capacity_m : nullptr); }
	const_iterator cbegin() const { return const_iterator(buckets, buckets ? buckets + capacity_m : nullptr); }
	const_iterator cend() const { return const_iterator(buckets ? buckets + capacity_m : nullptr, buckets ? buckets + capacity_m : nullptr); }

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
			resize(std::max(newCapacity, static_cast<size_t>(2)));
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
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, std::forward<P>(kv));

		++size_m;
	}

	void insert(pair&& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, std::move(kv));

		++size_m;
	}

	void insert(const pair& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, kv);

		++size_m;
	}

	template <typename P>
	void insert_or_assign(P&& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			entry->pair.second = std::forward<decltype(kv.second)>(kv.second);
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, std::forward<P>(kv));

		++size_m;
	}

	void insert_or_assign(pair&& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			entry->pair.second = std::move(kv.second);
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, std::move(kv));

		++size_m;
	}

	void insert_or_assign(const pair& kv)
	{
		reserve(size_m + 1);

		const auto& key = kv.first;
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			entry->pair.second = kv.second;
			return;
		}

		new (entry) Entry(EntryFlags::Filled, hash, kv);

		++size_m;
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		pair kv(std::forward<Args>(args)...);
		insert(std::move(kv));
	}

	iterator find(const TKey& key)
	{
		if (size_m == 0) return end();
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			return iterator(entry, buckets + capacity_m);
		}

		return end();
	}

	const_iterator find(const TKey& key) const
	{
		if (size_m == 0) return end();
		size_t hash = hasher(key);
		const Entry* entry = cfind_entry(buckets, capacity_m, key, hash);

		if (entry->isFilled())
		{
			return const_iterator(entry, buckets + capacity_m);
		}

		return end();
	}

	iterator erase(iterator it)
	{
		Entry& entry = *it.ptr;

		if (!entry.isFilled())
		{
			return end();
		}

		entry.~Entry();
		entry.flags = EntryFlags::Tombstone;

		++it;
		return it;
	}

	iterator erase(const TKey& key)
	{
		auto it = find(key);
		if (it != end())
		{
			return erase(it);
		}
		return end();
	}

	size_t erase_key(const TKey& key)
	{
		auto it = find(key);
		if (it != end())
		{
			erase(it);
			return 1;
		}
		return 0;
	}

	bool contains(const TKey& key) const
	{
		return find(key) != end();
	}

	void swap(dense_map& other) noexcept
	{
		std::swap(buckets, other.buckets);
		std::swap(capacity_m, other.capacity_m);
		std::swap(size_m, other.size_m);
		std::swap(allocator, other.allocator);
		std::swap(hasher, other.hasher);
		std::swap(key_equal, other.key_equal);
	}

	TValue& operator[](const TKey& key)
	{
		reserve(size_m + 1);
		size_t hash = hasher(key);
		Entry* entry = find_entry(buckets, capacity_m, key, hash);
		if (!entry->isFilled())
		{
			new (entry) Entry(EntryFlags::Filled, hash, pair(key, TValue{}));
			++size_m;
		}
		return entry->pair.second;
	}

	TValue& at(const TKey& key)
	{
		auto it = find(key);
		if (it == end()) throw std::out_of_range("dense_map::at: key not found");
		return it->second;
	}

	const TValue& at(const TKey& key) const
	{
		auto it = find(key);
		if (it == end()) throw std::out_of_range("dense_map::at: key not found");
		return it->second;
	}

	dense_map& operator=(const dense_map& other)
	{
		if (this != &other)
		{
			clear();
			allocator = other.allocator;
			hasher = other.hasher;
			if (other.size_m > 0)
			{
				resize(other.capacity_m);
				for (const auto& kv : other)
				{
					insert(kv);
				}
			}
		}
		return *this;
	}

	dense_map& operator=(dense_map&& other) noexcept
	{
		if (this != &other)
		{
			clear();
			allocator.deallocate(buckets, capacity_m);
			buckets = other.buckets;
			capacity_m = other.capacity_m;
			size_m = other.size_m;
			allocator = std::move(other.allocator);
			hasher = std::move(other.hasher);
			key_equal = std::move(other.key_equal);
			other.buckets = nullptr;
			other.capacity_m = 0;
			other.size_m = 0;
		}
		return *this;
	}
};

#endif