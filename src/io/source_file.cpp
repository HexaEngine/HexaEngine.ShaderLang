#include "source_file.hpp"

namespace HXSL
{
	std::string SourceFile::GetString(size_t start, size_t length)
	{
		return content.substr(start, length);
	}
}