#ifndef SPAN_HPP
#define SPAN_HPP

#include "pch/std.hpp"

namespace HXSL
{
	template <typename T>
	struct Span
	{
		static constexpr size_t npos = static_cast<size_t>(-1);

		const T* data;
		size_t length;

		Span() : data(nullptr), length(0) {}

		Span(const T* d, size_t l) : data(d), length(l) {}

		Span(const std::vector<T>& vec) : data(vec.data()), length(vec.size()) {}

		const T& operator[](size_t index) const
		{
			if (index >= length) {
				throw std::out_of_range("Index out of range in TextSpan");
			}
			return data[index];
		}

		Span<T> slice(size_t start) const
		{
			if (start >= length) throw std::out_of_range("Index out of range in Span");
			return Span<T>(data + start, length - start);
		}

		Span<T> slice(size_t start, size_t length) const
		{
			if (start + length > this->length) throw std::out_of_range("Slice exceeds span bounds");
			return Span<T>(data + start, length);
		}

		size_t find(const T& c) const noexcept
		{
			for (size_t i = 0; i < length; i++)
			{
				if (data[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		size_t find(const Span<T>& c) const noexcept
		{
			size_t len = c.length;
			if (len == 0) return 0;
			if (len > length) return npos;

			size_t x = 0;
			for (size_t i = 0; i < length; i++)
			{
				if (data[i] == c[x])
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

		const T* begin() const
		{
			return data;
		}

		const T* end() const
		{
			return data + length;
		}

		uint64_t fnv1a_64bit_hash() const noexcept
		{
			uint64_t hash = 14695981039346656037U;

			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
			const size_t stride = sizeof(T);
			const size_t end = length * stride;

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

		size_t indexOf(T c) const
		{
			for (size_t i = 0; i < length; i++)
			{
				if (data[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		size_t lastIndexOf(T c) const
		{
			if (length == 0) return npos;
			for (size_t i = length; i-- > 0; )
			{
				if (data[i] == c)
				{
					return i;
				}
			}
			return npos;
		}

		bool operator==(const Span<T>& other) const
		{
			if (this->length != other.length) return false;
			return std::memcmp(this->begin(), other.begin(), this->length * sizeof(T)) == 0;
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
			if (a.length != b.length) return false;
			return std::memcmp(a.begin(), b.begin(), a.length * sizeof(T)) == 0;
		}
	};

	using StringSpanHash = SpanHash<char>;
	using StringSpanEqual = SpanEqual<char>;

	struct StringSpan : public Span<char>
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

		StringSpan(const Span<char>& span) : Span(span.data, span.length)
		{
		}

		StringSpan(const std::string_view& span) : Span(span.data(), span.size())
		{
		}

		constexpr std::string_view view() const
		{
			return std::string_view(data, length);
		}

		std::string str() const
		{
			return std::string(data, length);
		}

		std::string str(size_t start) const
		{
			if (start > length) throw std::out_of_range("start index was out of range.");

			return std::string(data + start, length - start);
		}

		std::string str(size_t start, size_t length) const
		{
			if (start > length)
				throw std::out_of_range("start index was out of range.");

			if (start + length > this->length)
				throw std::out_of_range("substring length out of range.");

			return std::string(data + start, length);
		}

		bool operator==(const StringSpan& other) const
		{
			if (this->length != other.length) return false;
			return std::memcmp(this->begin(), other.begin(), this->length * sizeof(char)) == 0;
		}

		bool operator!=(const StringSpan& other) const
		{
			return !(*this == other);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const StringSpan& span)
	{
		return os.write(span.data, span.length);
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