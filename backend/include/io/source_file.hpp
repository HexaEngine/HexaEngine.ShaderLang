#ifndef SOURCE_FILE_HPP
#define SOURCE_FILE_HPP

#include "stream.hpp"
#include "source_location.hpp"
#include "text_stream.hpp"
#include "utils/span.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	class SourceManager;
	class SourceFile
	{
		SourceManager* srcManager;
		SourceFileID id;
		Stream* stream;
		bool closeStream;
		std::unique_ptr<TextStream> inputStream;

	public:
		SourceFile(SourceManager* srcManager, SourceFileID id, Stream* stream, bool closeStream) : srcManager(srcManager), id(id), stream(stream), closeStream(closeStream), inputStream(std::make_unique<TextStream>())
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

		SourceFileID GetID() const noexcept { return id; }

		bool PrepareInputStream();

		const std::unique_ptr<TextStream>& GetInputStream() const noexcept;

		void SetInputStream(std::unique_ptr<TextStream>&& value) noexcept;

		std::string GetString(size_t start, size_t length) const;

		StringSpan GetSpan(size_t start, size_t length) const;
	};
}

#endif