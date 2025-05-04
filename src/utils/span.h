#ifndef SPAN_H
#define SPAN_H

#include "lexical/text_span.hpp"

#include <stdexcept>
#include <sstream>

namespace HXSL
{
	template <typename T>
	struct Span
	{
		static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

		const T* data;
		size_t start;
		size_t length;

		Span(const T* d, size_t s, size_t l) : data(d), start(s), length(l) {}

		size_t End() const
		{
			return start + length;
		}

		const char& operator[](size_t index) const
		{
			if (index >= length) {
				throw std::out_of_range("Index out of range in TextSpan");
			}
			return data[start + index];
		}

		Span<T> slice(size_t start) const
		{
			if (start >= length) throw std::out_of_range("Index out of range in Span");
			return Span<T>(data, this->start + start, length - start);
		}

		Span<T> slice(size_t start, size_t length) const
		{
			if (start + length > this->length) throw std::out_of_range("Slice exceeds span bounds");
			return Span<T>(data, this->start + start, length);
		}

		size_t find(const T& c) const noexcept
		{
			for (int i = 0; i < length; i++)
			{
				if (data[start + i] == c)
				{
					return i;
				}
			}
			return std::string::npos;
		}

		size_t find(const Span<T>& c) const noexcept
		{
			size_t len = c.length;
			if (len == 0) return 0;
			if (len > length) return std::string::npos;

			size_t x = 0;
			for (size_t i = 0; i < length; i++)
			{
				if (data[start + i] == c[x])
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
			return std::string::npos;
		}

		const char* begin() const
		{
			return data + start;
		}

		const char* end() const
		{
			return data + start + length;
		}

		uint64_t fnv1a_64bit_hash() const noexcept
		{
			uint64_t hash = 14695981039346656037U;

			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
			const size_t stride = sizeof(T);
			const size_t offset = start * stride;
			const size_t end = (start + length) * stride;

			for (size_t i = offset; i < end; i++)
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
				if (data[start + i] == c)
				{
					return i;
				}
			}
			return std::string::npos;
		}

		size_t lastIndexOf(T c) const
		{
			if (length == 0) return std::string::npos;
			for (size_t i = length; i-- > 0; )
			{
				if (data[start + i] == c)
				{
					return i;
				}
			}
			return std::string::npos;
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
		StringSpan() : Span(nullptr, 0, 0)
		{
		}

		StringSpan(const std::string& str) : Span(str.c_str(), 0, str.length())
		{
		}

		StringSpan(const char* str) : Span(str, 0, strlen(str))
		{
		}

		StringSpan(const char* str, size_t start, size_t length) : Span(str, start, length)
		{
		}

		StringSpan(const Span<char>& span) : Span(span.data, span.start, span.length)
		{
		}

		StringSpan(const std::string_view& span) : Span(span.data(), 0, span.size())
		{
		}

		std::string str() const
		{
			return std::string(data + start, length);
		}
	};
}

#endif