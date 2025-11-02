#ifndef SPAN_HPP
#define SPAN_HPP

#include "pch/std.hpp"

namespace HXSL
{
	template <typename T>
	class Span
	{
	public:
		static constexpr bool IsConst = std::is_const_v<T>;
		using TClean = std::remove_const_t<T>;
		using TPtr = std::conditional_t<IsConst, const TClean*, TClean*>;
		using TRef = std::conditional_t<IsConst, const TClean&, TClean&>;
		using TConstRef = const TClean&;
		using TConstPtr = const TClean*;

		static constexpr size_t npos = static_cast<size_t>(-1);
		using iterator = TPtr;
		using const_iterator = TConstPtr;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	protected:
		TPtr data_m;
		size_t size_m;

	public:
		Span() : data_m(nullptr), size_m(0) {}

		Span(TPtr d, size_t size) : data_m(d), size_m(size) {}

		Span(std::vector<TClean>& vec) : data_m(vec.data()), size_m(vec.size()) {}

		TPtr data() { return data_m; }
		TConstPtr data() const { return data_m; }
		size_t size() const { return size_m; }
		bool empty() const { return size_m == 0; }

		TRef operator[](size_t index)
		{
			return data_m[index];
		}

		TConstRef operator[](size_t index) const
		{
			return data_m[index];
		}

		Span<T> slice(size_t start) const
		{
			if (start >= size_m) throw std::out_of_range("Index out of range in Span");
			return Span<T>(data_m + start, size_m - start);
		}

		Span<T> slice(size_t start, size_t length) const
		{
			if (start + length > this->size_m) throw std::out_of_range("Slice exceeds span bounds");
			return Span<T>(data_m + start, length);
		}

		size_t find(const T& c) const noexcept
		{
			for (size_t i = 0; i < size_m; i++)
			{
				if (data_m[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		size_t find(const Span<T>& c) const noexcept
		{
			size_t len = c.size_m;
			if (len == 0) return 0;
			if (len > size_m) return npos;

			size_t x = 0;
			for (size_t i = 0; i < size_m; i++)
			{
				if (data_m[i] == c[x])
				{
					x++;
					if (x == len)
					{
						return i - x + 1;
					}
				}
				else
				{
					x = 0;
				}
			}
			return npos;
		}

		iterator begin() { return data_m; }
		iterator end() { return data_m + size_m; }
		const_iterator begin() const { return data_m; }
		const_iterator end() const { return data_m + size_m; }
		const_iterator cbegin() const { return data_m; }
		const_iterator cend() const { return data_m + size_m; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
		const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

		uint64_t fnv1a_64bit_hash() const noexcept
		{
			uint64_t hash = 14695981039346656037U;

			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data_m);
			const size_t stride = sizeof(T);
			const size_t end = size_m * stride;

			for (size_t i = 0; i < end; ++i)
			{
				hash ^= static_cast<uint64_t>(ptr[i]);
				hash *= 1099511628211U;
			}

			return hash;
		}

		uint64_t hash() const noexcept
		{
			return fnv1a_64bit_hash();
		}

		size_t indexOf(const TClean& c) const
		{
			for (size_t i = 0; i < size_m; i++)
			{
				if (data_m[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		size_t lastIndexOf(const TClean& c) const
		{
			if (size_m == 0) return npos;
			for (size_t i = size_m; i-- > 0; )
			{
				if (data_m[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		bool operator==(const Span<T>& other) const
		{
			if (this->size_m != other.size_m) return false;
			return std::memcmp(this->begin(), other.begin(), this->size_m * sizeof(T)) == 0;
		}

		bool operator!=(const Span<T>& other) const
		{
			return !(*this == other);
		}
	};

	template <typename T>
	struct SpanHash
	{
		size_t operator()(const Span<T>& span) const noexcept
		{
			return static_cast<size_t>(span.hash());
		}
	};

	template <typename T>
	struct SpanEqual
	{
		bool operator()(const Span<T>& a, const Span<T>& b) const noexcept
		{
			if (a.size() != b.size()) return false;
			return std::memcmp(a.begin(), b.begin(), a.size() * sizeof(T)) == 0;
		}
	};

	using StringSpanHash = SpanHash<const char>;
	using StringSpanEqual = SpanEqual<const char>;

	struct StringSpan : public Span<const char>
	{
		StringSpan() : Span(nullptr, 0)
		{
		}

		StringSpan(const std::string& str) : Span(str.c_str(), str.length())
		{
		}

		StringSpan(const char* str) : Span(str, strlen(str))
		{
		}

		StringSpan(const char* str, size_t length) : Span(str, length)
		{
		}

		StringSpan(const char* str, size_t start, size_t length) : Span(str + start, length)
		{
		}

		StringSpan(const Span<char>& span) : Span(span.data(), span.size())
		{
		}

		StringSpan(const std::string_view& span) : Span(span.data(), span.size())
		{
		}

		constexpr std::string_view view() const
		{
			return std::string_view(data_m, size_m);
		}

		std::string str() const
		{
			return std::string(data_m, size_m);
		}

		std::string str(size_t start) const
		{
			if (start > size_m) throw std::out_of_range("start index was out of range.");

			return std::string(data_m + start, size_m - start);
		}

		std::string str(size_t start, size_t length) const
		{
			if (start > length)
				throw std::out_of_range("start index was out of range.");

			if (start + length > this->size_m)
				throw std::out_of_range("substring length out of range.");

			return std::string(data_m + start, length);
		}

		bool operator==(const StringSpan& other) const
		{
			if (this->size_m != other.size_m) return false;
			return std::memcmp(this->begin(), other.begin(), this->size_m * sizeof(char)) == 0;
		}

		bool operator!=(const StringSpan& other) const
		{
			return !(*this == other);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const StringSpan& span)
	{
		return os.write(span.data(), span.size());
	}

	template <typename T>
	class ArrayRef
	{
	public:
		using iterator = T*;
		using const_iterator = const T*;
		using pointer = T*;
		using const_pointer = T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = size_t;
	private:
		T* data_m;
		size_type size_m;

	public:
		ArrayRef() : data_m(nullptr), size_m(0) {}
		ArrayRef(T* data, size_type size) : data_m(data), size_m(size)
		{
		}
		ArrayRef(std::vector<T>& vec) : data_m(vec.data()), size_m(vec.size())
		{
		}

		pointer data() noexcept { return data_m; }
		size_type size() const noexcept { return size_m; }
		bool empty() const noexcept { return size_m == 0; }

		const_reference operator[](size_type index) const
		{
			if (index >= size_m)
			{
				throw std::out_of_range("Index out of range");
			}
			return data_m[index];
		}

		reference operator[](size_type index)
		{
			if (index >= size_m)
			{
				throw std::out_of_range("Index out of range");
			}
			return data_m[index];
		}

		iterator begin() noexcept { return data_m; }
		iterator end() noexcept { return data_m + size_m; }
		const_iterator begin() const noexcept { return data_m; }
		const_iterator end() const noexcept { return data_m + size_m; }
		const_iterator cbegin() const noexcept { return data_m; }
		const_iterator cend() const noexcept { return data_m + size_m; }

		void init()
		{
			if (std::is_trivially_copyable<T>::value)
			{
				std::memset(data_m, 0, size_m);
			}
			else
			{
				std::uninitialized_value_construct(begin(), end());
			}
		}

		void clear()
		{
			if (std::is_trivially_copyable<T>::value)
			{
				std::memset(data_m, 0, size_m);
			}
			else
			{
				std::destroy(begin(), end());
			}
		}
	};
}

namespace std
{
	template <typename T>
	struct hash<HXSL::Span<T>>
	{
		size_t operator()(const HXSL::Span<T>& span) const noexcept
		{
			return static_cast<size_t>(span.hash());
		}
	};

	template <>
	struct hash<HXSL::StringSpan>
	{
		size_t operator()(const HXSL::StringSpan& span) const noexcept
		{
			return static_cast<size_t>(span.hash());
		}
	};
}

#endif