#ifndef TEXT_SPAN_HPP
#define TEXT_SPAN_HPP

#include "config.h"
#include "io/source_file.hpp"
#include <stdexcept>
#include <string>
#include <sstream>

namespace HXSL
{
	struct TextSpan
	{
		SourceFile* source;
		size_t start;
		size_t length;
		uint32_t line;
		uint32_t column;

		TextSpan() : source(nullptr), start(0), length(0), line(1), column(1) {}

		TextSpan(SourceFile* source, size_t start, size_t length, uint32_t line, uint32_t column) : source(source), start(start), length(length), line(line), column(column)
		{
		}

		size_t End() const
		{
			return start + length;
		}

		TextSpan merge(TextSpan other) const
		{
			if (source != other.source)
			{
				//HXSL_ASSERT(false, "Cannot merge TextSpan based of a different string pointer.");
				return {};
			}

			size_t newStart = std::min(start, other.start);
			size_t newEnd = std::max(End(), other.End());
			size_t newLength = newEnd - newStart;

			uint32_t newLine = line;
			uint32_t newColumn = column;

			if (other.line < line)
			{
				newLine = other.line;
				newColumn = other.column;
			}
			else if (other.line == line)
			{
				newColumn = std::min(column, other.column);
			}
			else
			{
				newColumn = other.column;
			}

			return TextSpan(source, newStart, newLength, newLine, newColumn);
		}

		std::string str() const
		{
			return source ? source->GetString(start, length) : "";
		}
	};
}

#endif