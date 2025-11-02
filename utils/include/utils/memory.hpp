#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "pch/std.hpp"

namespace HXSL
{
	template<typename T>
	static constexpr T alignTo(T size, size_t alignment)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	template<typename T>
	static constexpr T AlignUp(T size, size_t alignment)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	template<typename T>
	struct aligned_allocator
	{
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

		static constexpr std::size_t default_alignment = alignof(T);

		aligned_allocator() noexcept = default;

		template<typename U>
		aligned_allocator(const aligned_allocator<U>&) noexcept {}

		template<typename U>
		struct rebind
		{
			using other = aligned_allocator<U>;
		};

		[[nodiscard]] T* allocate(std::size_t n)
		{
			return allocate(n, default_alignment);
		}

		[[nodiscard]] T* allocate(std::size_t n, std::size_t alignment)
		{
			if (n == 0) return nullptr;

			if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
				throw std::invalid_argument("Alignment must be a power of 2");
			}

			alignment = std::max(alignment, alignof(T));

			constexpr std::size_t max_size = std::numeric_limits<std::size_t>::max() / sizeof(T);
			if (n > max_size) {
				throw std::bad_array_new_length();
			}

			void* ptr = nullptr;
			std::size_t size = n * sizeof(T);

#if defined(_MSC_VER)
			ptr = _aligned_malloc(size, alignment);
			if (!ptr) throw std::bad_alloc();
#elif defined(__unix__) || defined(__APPLE__)
			if (posix_memalign(&ptr, alignment, size) != 0) {
				throw std::bad_alloc();
			}
#else
			std::size_t aligned_size = ((size + alignment - 1) / alignment) * alignment;
			ptr = std::aligned_alloc(alignment, aligned_size);
			if (!ptr) throw std::bad_alloc();
#endif
			return static_cast<T*>(ptr);
		}

		void deallocate(T* ptr, std::size_t) noexcept
		{
			if (ptr == nullptr) return;

#if defined(_MSC_VER)
			_aligned_free(ptr);
#else
			std::free(ptr);
#endif
		}

		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
		}

		template<typename U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}

		template<typename U>
		bool operator==(const aligned_allocator<U>&) const noexcept { return true; }

		template<typename U>
		bool operator!=(const aligned_allocator<U>&) const noexcept { return false; }

		std::size_t max_size() const noexcept
		{
			return std::numeric_limits<std::size_t>::max() / sizeof(T);
		}

	private:
		static constexpr std::size_t align_up(std::size_t size, std::size_t alignment) noexcept
		{
			return ((size + alignment - 1) / alignment) * alignment;
		}
	};

	template<typename T>
	using simd_allocator = aligned_allocator<T>;

	template<std::size_t Alignment, typename T>
	using aligned_allocator_t = aligned_allocator<T>;
}

#endif