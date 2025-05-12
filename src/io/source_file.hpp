#ifndef SOURCE_FILE_HPP
#define SOURCE_FILE_HPP

#include "io/stream.hpp"
#include "utils/span.hpp"

#include <string>
#include <memory>

namespace HXSL
{
	class LexerStream;

	class SourceFile
	{
		Stream* stream;
		bool closeStream;
		std::unique_ptr<LexerStream> inputStream;

	public:
		SourceFile(Stream* stream, bool closeStream) : stream(stream), closeStream(closeStream), inputStream(std::make_unique<LexerStream>())
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

		const std::unique_ptr<LexerStream>& GetInputStream() const noexcept;

		void SetInputStream(std::unique_ptr<LexerStream>&& value) noexcept;

		std::string GetString(size_t start, size_t length);

		StringSpan GetSpan(size_t start, size_t length);
	};
}

#endif