#ifndef SOURCE_FILE_HPP
#define SOURCE_FILE_HPP

#include "io/stream.hpp"
#include "lexical/input_stream.hpp"

#include <string>
#include <memory>

namespace HXSL
{
	using LexerInputStream = Lexer::InputStream;

	class SourceFile
	{
		Stream* stream;
		bool closeStream;
		std::unique_ptr<LexerInputStream> inputStream;

	public:
		SourceFile(Stream* stream, bool closeStream) : stream(stream), closeStream(closeStream), inputStream(std::make_unique<LexerInputStream>())
		{
		}

		~SourceFile()
		{
			if (stream)
			{
				if (closeStream)
				{
					delete stream;
				}
				stream = nullptr;
			}
		}

		bool PrepareInputStream();

		LexerInputStream* GetInputStream() const noexcept
		{
			return inputStream.get();
		}

		std::string GetString(size_t start, size_t length);
	};
}

#endif