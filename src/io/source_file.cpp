#include "source_file.hpp"
#include "lexical/token.hpp"
#include "lexical/input_stream.hpp"

namespace HXSL
{
	bool SourceFile::PrepareInputStream()
	{
		return inputStream->CopyFrom(*stream);
	}

	const std::unique_ptr<LexerStream>& SourceFile::GetInputStream() const noexcept
	{
		return inputStream;
	}

	void SourceFile::SetInputStream(std::unique_ptr<LexerStream>&& value) noexcept
	{
		inputStream = std::move(value);
	}

	std::string SourceFile::GetString(size_t start, size_t length)
	{
		auto buffer = inputStream->GetBuffer();
		auto bufferSize = inputStream->GetLength();

		if (start >= bufferSize || length == 0)
		{
			return {};
		}

		size_t toCopy = std::min(length, bufferSize - start);

		if (toCopy == 0)
		{
			return {};
		}

		std::string result;
		result.resize(toCopy);
		std::memcpy(result.data(), buffer + start, toCopy);

		return result;
	}

	StringSpan HXSL::SourceFile::GetSpan(size_t start, size_t length)
	{
		auto buffer = inputStream->GetBuffer();
		auto bufferSize = inputStream->GetLength();

		if (start >= bufferSize || length == 0)
		{
			return {};
		}

		size_t actualLength = std::min(length, bufferSize - start);

		return StringSpan(buffer, start, actualLength);
	}
}