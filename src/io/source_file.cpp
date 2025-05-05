#include "source_file.hpp"
#include "source_file.hpp"
#include "source_file.hpp"

namespace HXSL
{
	bool SourceFile::PrepareInputStream()
	{
		return inputStream->CopyFrom(*stream);
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
}