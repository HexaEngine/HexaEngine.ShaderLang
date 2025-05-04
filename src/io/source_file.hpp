#ifndef SOURCE_FILE_HPP
#define SOURCE_FILE_HPP

namespace HXSL
{
	class SourceFile
	{
		const char* ptr;

	public:

		void GetString(size_t start, size_t length, const char*& textOut);
	};
}

#endif