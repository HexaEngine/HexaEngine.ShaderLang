#ifndef SOURCE_FILE_HPP
#define SOURCE_FILE_HPP

#include <string>

namespace HXSL
{
	class SourceFile
	{
		std::string content;

	public:
		SourceFile(const std::string& content) : content(content)
		{
		}

		const std::string& GetContent() const { return content; }

		std::string GetString(size_t start, size_t length);
	};
}

#endif