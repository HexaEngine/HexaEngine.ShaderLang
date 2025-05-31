#ifndef BUMP_ALLOCATOR_HPP
#define BUMP_ALLOCATOR_HPP

#include "pch/std.hpp"
#include "span.hpp"

namespace HXSL
{
	class BumpAllocator
	{
		static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

		inline static size_t AlignUp(size_t size, size_t alignment)
		{
			return (size + alignment - 1) & ~(alignment - 1);
		}

		struct Block
		{
			char* memory;
			size_t size;
			size_t used;

			Block(size_t size) : size(size), used(0)
			{
				memory = new char[size];
			}

			~Block()
			{
				delete[] memory;
			}

			Block(const Block&) = delete;

			Block& operator=(Block&& other) noexcept
			{
				if (this != &other)
				{
					if (memory)
					{
						delete[] memory;
					}

					memory = other.memory;
					size = other.size;
					used = other.used;

					other.memory = nullptr;
					other.size = 0;
					other.used = 0;
				}
				return *this;
			}

			Block(Block&& other) noexcept : memory(other.memory), size(other.size), used(other.used)
			{
				other.memory = nullptr;
				other.size = 0;
				other.used = 0;
			}

			void* Alloc(size_t bytes, size_t alignment) noexcept
			{
				size_t base = AlignUp(used, alignment);
				size_t newUsed = base + bytes;
				if (newUsed > size)
				{
					return nullptr;
				}

				used = newUsed;
				return memory + base;
			}
		};

		std::vector<Block> blocks;

	public:
		BumpAllocator() = default;

		BumpAllocator(const BumpAllocator& other) = delete;
		BumpAllocator operator=(BumpAllocator other) = delete;

		BumpAllocator(BumpAllocator&& other) noexcept : blocks(std::move(other.blocks)) {}
		BumpAllocator& operator=(BumpAllocator&& other) noexcept
		{
			if (this != &other)
			{
				blocks = std::move(other.blocks);
			}
			return *this;
		}

		~BumpAllocator()
		{
			ReleaseAll();
		}

		void* Alloc(size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept
		{
			if (blocks.size() != 0)
			{
				auto& top = blocks.back();
				auto ptr = top.Alloc(bytes, alignment);
				if (ptr != nullptr)
				{
					return ptr;
				}
			}

			size_t blockSize = AlignUp(bytes * 2, DEFAULT_BLOCK_SIZE);
			blocks.emplace_back(blockSize);
			return blocks.back().Alloc(bytes, alignment);
		}

		void Reset() noexcept
		{
			for (auto& b : blocks)
			{
				b.used = 0;
			}
		}

		void ReleaseAll() noexcept
		{
			blocks.clear();
			blocks.shrink_to_fit();
		}

		template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* Alloc(_Types&&... args)
		{
			void* rawMem = Alloc(sizeof(_Ty), alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(std::forward<_Types>(args)...);
		}

		template <class _Ty, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* Alloc(_Ty&& val)
		{
			void* rawMem = Alloc(sizeof(_Ty), alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(std::forward<_Ty>(val));
		}

		template <class _Ty, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* Alloc(const _Ty& val)
		{
			void* rawMem = Alloc(sizeof(_Ty), alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(val);
		}

		template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* AllocAddit(size_t additional, _Types&&... args)
		{
			void* rawMem = Alloc(sizeof(_Ty) + additional, alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(std::forward<_Types>(args)...);
		}

		template <class _Ty, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* AllocAddit(size_t additional, _Ty&& val)
		{
			void* rawMem = Alloc(sizeof(_Ty) + additional, alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(std::forward<_Ty>(val));
		}

		template <class _Ty, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
		_CONSTEXPR23 _Ty* AllocAddit(size_t additional, const _Ty& val)
		{
			void* rawMem = Alloc(sizeof(_Ty) + additional, alignof(_Ty));
			if (!rawMem) throw std::bad_alloc{};
			return new(rawMem) _Ty(val);
		}

		StringSpan CopyString(const StringSpan& span)
		{
			if (span.length == 0) return {};
			auto copyPtr = reinterpret_cast<char*>(Alloc((span.length + 1) * sizeof(char), alignof(char)));
			std::memcpy(copyPtr, span.data, span.length * sizeof(char));
			copyPtr[span.length] = '\0';
			return StringSpan(copyPtr, span.length);
		}

		template<typename T>
		Span<T> CopySpan(const Span<T>& span)
		{
			if (span.length == 0) return {};
			auto copied = reinterpret_cast<T*>(Alloc(span.length * sizeof(T), alignof(T)));
			std::memcpy(copied, span.data, span.length * sizeof(T));
			return Span<T>(copied, span.length);
		}

		template<typename T>
		Span<T> CopySpan(const std::vector<T>& span)
		{
			if (span.size() == 0) return {};
			auto copied = reinterpret_cast<T*>(Alloc(span.size() * sizeof(T), alignof(T)));
			std::memcpy(copied, span.data(), span.size() * sizeof(T));
			return Span<T>(copied, span.size());
		}
	};

	template <typename T>
	class StdBumpAllocator
	{
		BumpAllocator* bumpAllocator = nullptr;
	public:
		using value_type = T;

		StdBumpAllocator() noexcept = default;

		StdBumpAllocator(BumpAllocator& alloc) noexcept : bumpAllocator(&alloc) {}

		template <typename U>
		StdBumpAllocator(const StdBumpAllocator<U>& other) noexcept : bumpAllocator(other.bumpAllocator) {}

		T* allocate(std::size_t n)
		{
			void* ptr = bumpAllocator->Alloc(n * sizeof(T), alignof(T));
			if (!ptr) throw std::bad_alloc{};
			return static_cast<T*>(ptr);
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
		}

		inline operator BumpAllocator& () const
		{
			assert(bumpAllocator != nullptr);
			return *bumpAllocator;
		}

		template <typename U>
		inline bool operator==(const StdBumpAllocator<U>& other) const noexcept
		{
			return bumpAllocator == other.bumpAllocator;
		}

		template <typename U>
		inline bool operator!=(const StdBumpAllocator<U>& other) const noexcept
		{
			return !(*this == other);
		}
	};
}

#endif