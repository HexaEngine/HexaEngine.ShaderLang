#ifndef TEXT_MAPPING_HPP
#define TEXT_MAPPING_HPP

#include "io/source_file.hpp"
#include "input_stream.hpp"

namespace HXSL
{
	struct TextMapping
	{
		SourceFile* file = nullptr;
		size_t start;
		size_t length;

		int32_t lineOffset;
		int32_t columnOffset;

		TextMapping(SourceFile* file, size_t start, size_t length,  int32_t lineOffset, int32_t columnOffset) :file(file), start(start), length(length), lineOffset(lineOffset), columnOffset(columnOffset)
		{
		}

		TextMapping() = default;
	};
}

#endif