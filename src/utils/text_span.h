#ifndef TEXT_SPAN_H
#define TEXT_SPAN_H

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
		uint32_t Line;
		uint32_t Column;

		TextSpan() : Text(nullptr), Start(0), Length(0), Line(1), Column(1) {}

		TextSpan(const char* text, size_t start, size_t length, uint32_t line, uint32_t column) : Text(text), Start(start), Length(length), Line(line), Column(column)
		{
		}

		TextSpan(const std::string& str) : Text(str.c_str()), Start(0), Length(str.length()), Line(0), Column(0)
		{
		}

		size_t End() const
		{
			return Start + Length;
		}

		TextSpan merge(TextSpan other) const
		{
			if (Text != other.Text)
			{
				//HXSL_ASSERT(false, "Cannot merge TextSpan based of a different string pointer.");
				return {};
			}

			size_t newStart = std::min(Start, other.Start);
			size_t newEnd = std::max(End(), other.End());
			size_t newLength = newEnd - newStart;

			uint32_t newLine = Line;
			uint32_t newColumn = Column;

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
	};
}

#endif