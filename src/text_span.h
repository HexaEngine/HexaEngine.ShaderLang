#ifndef SPAN_H
#define SPAN_H

#include "config.h"
#include <stdexcept>
#include <string>
#include <sstream>

namespace HXSL
{
	struct TextSpan
	{
		const char* Text;
		size_t Start;
		size_t Length;
		size_t Line;
		size_t Column;

		TextSpan() : Text(nullptr), Start(0), Length(0), Line(1), Column(1) {}

		TextSpan(const char* text, size_t start, size_t length, size_t line, size_t column) : Text(text), Start(start), Length(length), Line(line), Column(column)
		{
		}

		TextSpan(const std::string& str) : Text(str.c_str()), Start(0), Length(str.length()), Line(0), Column(0)
		{
		}

		size_t End() const
		{
			return Start + Length;
		}

		const char& operator[](size_t index) const
		{
			if (index < 0 || index >= Length) {
				throw std::out_of_range("Index out of range in TextSpan");
			}
			return Text[Start + index];
		}

		TextSpan merge(TextSpan other) const
		{
			if (Text != other.Text)
			{
				HXSL_ASSERT(false, "Cannot merge TextSpan based of a different string pointer.");
				return {};
			}

			size_t newStart = std::min(Start, other.Start);
			size_t newEnd = std::max(End(), other.End());
			size_t newLength = newEnd - newStart;

			size_t newLine = Line;
			size_t newColumn = Column;

			if (other.Line < Line)
			{
				newLine = other.Line;
				newColumn = other.Column;
			}
			else if (other.Line == Line)
			{
				newColumn = std::min(Column, other.Column);
			}
			else
			{
				newColumn = other.Column;
			}

			return TextSpan(Text, newStart, newLength, newLine, newColumn);
		}

		TextSpan slice(size_t start)
		{
			return TextSpan(Text, Start + start, Length - start, Line, Column);
		}

		TextSpan slice(size_t start, size_t length)
		{
			return TextSpan(Text, Start + start, length, Line, Column);
		}

		size_t find(const char& c) const noexcept
		{
			for (int i = 0; i < Length; i++)
			{
				if (Text[Start + i] == c)
				{
					return i;
				}
			}
			return std::string::npos;
		}

		size_t find(const std::string& c) const noexcept
		{
			size_t len = c.length();
			if (len == 0) return 0;
			if (len > Length) return std::string::npos;

			size_t x = 0;
			for (size_t i = 0; i < Length; i++)
			{
				if (Text[Start + i] == c[x])
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

		const char* begin() const {
			return Text + Start;
		}

		const char* end() const {
			return Text + Start + Length;
		}

		uint64_t fnv1a_64bit_hash() const noexcept
		{
			uint64_t hash = 14695981039346656037U;
			for (size_t i = 0; i < Length; i++)
			{
				hash ^= static_cast<uint64_t>(Text[Start + i]);
				hash *= 1099511628211U;
			}
			return hash;
		}

		uint64_t hash() const noexcept
		{
			return fnv1a_64bit_hash();
		}

		std::string toString() const
		{
			return std::string(Text + Start, Length);
		}

		size_t indexOf(char c) const
		{
			for (size_t i = 0; i < Length; i++)
			{
				if (Text[Start + i] == c)
				{
					return i;
				}
			}
			return -1;
		}

		bool operator==(const TextSpan& other) const
		{
			if (this->Length != other.Length) return false;
			return std::memcmp(this->begin(), other.begin(), this->Length) == 0;
		}
	};

	struct TextSpanHash
	{
		size_t operator()(const TextSpan& span) const noexcept
		{
			return static_cast<size_t>(span.hash());
		}
	};

	struct TextSpanEqual
	{
		bool operator()(const TextSpan& a, const TextSpan& b) const noexcept
		{
			if (a.Length != b.Length) return false;
			return std::memcmp(a.begin(), b.begin(), a.Length) == 0;
		}
	};
}

#endif