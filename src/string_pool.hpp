#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <string>
#include "span.h"

namespace HXSL
{
	using StringSpanHash = SpanHash<char>;
	using StringSpanEqual = SpanEqual<char>;

	struct StringSpan : public Span<char>
	{
		StringSpan(const std::string& str) : Span(str.c_str(), 0, str.length())
		{
		}

		StringSpan(const char* str) : Span(str, 0, strlen(str))
		{
		}

		std::string str() const
		{
			return std::string(data + start, length);
		}
	};

	class StringPool
	{
	private:
		std::vector<std::unique_ptr<std::string>> strings;
		std::unordered_map<StringSpan, size_t, StringSpanHash, StringSpanEqual> stringToIndex;

	public:
		const std::string& add(const std::string& str)
		{
			StringSpan view = str;

			auto it = stringToIndex.find(view);
			if (it != stringToIndex.end())
			{
				return *strings[it->second].get();
			}

			auto r = std::make_unique<std::string>(str);
			auto idx = strings.size();
			auto strPtr = r.get();

			strings.push_back(std::move(r));
			stringToIndex.insert(std::make_pair(*strPtr, idx));

			return *strPtr;
		}

		const std::string& add(const char* str)
		{
			StringSpan view = str;

			auto it = stringToIndex.find(view);
			if (it != stringToIndex.end())
			{
				return *strings[it->second].get();
			}

			auto r = std::make_unique<std::string>(str);
			auto idx = strings.size();
			auto strPtr = r.get();

			strings.push_back(std::move(r));
			stringToIndex.insert(std::make_pair(*strPtr, idx));

			return *strPtr;
		}

		const char* get_cstr(size_t index) const
		{
			if (index < strings.size())
			{
				return strings[index]->c_str();
			}
			return nullptr;
		}

		void clear()
		{
			strings.clear();
			stringToIndex.clear();
		}

		~StringPool()
		{
		}
	};
}
#endif