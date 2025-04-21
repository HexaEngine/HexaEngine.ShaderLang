#ifndef TEXT_HELPER_H
#define TEXT_HELPER_H
#include <cctype>
#include <string>
#include "text_helper.h"
#include <unordered_set>

namespace TextHelper
{
	bool IsLetterOrDigit(char c)
	{
		return std::isalpha(c) || std::isdigit(c);
	}

	bool MatchPair(const char* text, size_t index, size_t length, char current, char first, char second)
	{
		return current == first && index + 1 < length && *(text + index + 1) == second;
	}

	size_t FindEnd(size_t start, const char* text, size_t length, bool (*selector)(char))
	{
		size_t position = start;
		while (position < length && selector(text[position]))
		{
			position++;
		}
		return position;
	}

	size_t FindEndOfLine(const char* text, size_t offset, size_t length)
	{
		const char* current = text + offset;
		const char* end = text + length;

		while (current != end)
		{
			char c = *current;
			if (c == '\n' || c == '\r')
			{
				if (current + 1 != end)
				{
					current++;
					c = *current;
					if (c != '\n' && c != '\r')
					{
						current--;
					}
				}
				return (size_t)(current - text - offset + 1);
			}
			current++;
		}

		return length - offset;
	}

	size_t FindWordBoundary(const char* text, size_t offset, size_t length)
	{
		const char* current = text + offset;
		const char* end = text + length;

		while (current != end)
		{
			char c = *current;
			if (!IsLetterOrDigit(c) && c != '_')
			{
				return (size_t)(current - text - offset);
			}
			current++;
		}

		return length - offset;
	}

	size_t FindOperatorBoundary(const char* text, size_t offset, size_t length, std::unordered_set<char> delimiter)
	{
		const char* current = text + offset;
		const char* end = text + length;

		while (current != end)
		{
			char c = *current;
			if (std::isspace(c) || std::isdigit(c) || std::isalpha(c) || delimiter.find(c) != delimiter.end())
			{
				return (size_t)(current - text - offset);
			}
			current++;
		}

		return length - offset;
	}

	size_t FindWhitespaceWordBoundary(const char* text, size_t offset, size_t length)
	{
		const char* current = text + offset;
		const char* end = text + length;

		while (current != end)
		{
			if (std::isspace(*current))
			{
				return (size_t)(current - text - offset);
			}
			current++;
		}

		return length - offset;
	}

	bool LookAhead(const char* text, size_t offset, size_t length, char target, size_t& trackedLength)
	{
		const char* current = text + offset;
		const char* end = text + length;

		bool escaped = false;
		while (current < end)
		{
			char c = *current;
			if (c == target && !escaped)
			{
				trackedLength = (size_t)(current - text - offset);
				return true;
			}
			escaped = false;
			if (c == '\\')
			{
				escaped = true;
			}
			current++;
		}
		trackedLength = 0;
		return false;
	}

	bool LookAhead(const char* text, size_t offset, size_t length, const std::string& target, size_t& trackedLength)
	{
		const char* current = text + offset;
		const char* end = text + length;

		bool escaped = false;
		size_t ix = 0;
		while (current < end)
		{
			char c = *current;
			if (c == target[ix] && !escaped)
			{
				ix++;
				if (ix == target.length())
				{
					trackedLength = (size_t)(current - text - offset);
					return true;
				}
			}
			else
			{
				ix = 0;
			}

			escaped = false;
			if (c == '\\')
			{
				escaped = true;
			}
			current++;
		}
		trackedLength = 0;
		return false;
	}

	bool StartsWith(const char* text, size_t offset, size_t length, const std::string& value)
	{
		size_t valueLength = value.length();
		if (length - offset < valueLength)
			return false;

		const char* pat = value.c_str();
		const char* current = text + offset;
		const char* end = pat + valueLength;
		const char* patCurrent = pat;
		while (patCurrent != end)
		{
			if (*current != *patCurrent)
			{
				return false;
			}
			current++;
			patCurrent++;
		}

		return true;
	}

	size_t CountLeadingWhitespace(const char* text, size_t length)
	{
		const char* current = text;
		const char* end = text + length;
		while (current != end && std::isspace(*current))
		{
			current++;
		}
		return (size_t)(current - text);
	}

	size_t CountLeadingWhitespace(const char* text)
	{
		const char* current = text;
		char c;
		while ((c = *current) != '\0' && std::isspace(c))
		{
			current++;
		}
		return (size_t)(current - text);
	}

	size_t CountTrailingWhitespace(const char* text, size_t length)
	{
		const char* current = text + length - 1;
		const char* start = text;
		while (current >= start && std::isspace(*current))
		{
			current--;
		}
		return (size_t)(text + length - 1 - current);
	}

	void SkipLeadingWhitespace(const char* text, size_t& index, size_t length)
	{
		index += CountLeadingWhitespace(text + index, length - index);
	}

	void SkipLeadingWhitespace(const char*& text, size_t length)
	{
		text += CountLeadingWhitespace(text, length);
	}

	void SkipTrailingWhitespace(const char* text, size_t index, size_t& length)
	{
		length -= CountTrailingWhitespace(text + index, length - index);
	}

	void SkipTrailingWhitespace(const char* text, size_t& length)
	{
		length -= CountTrailingWhitespace(text, length);
	}

	bool TryParseIdentifier(const char* text, size_t offset, size_t length, size_t& trackedLength)
	{
		const char* current = text + offset;
		const char* end = text + length;
		if (current == end || !std::isalpha(*current) && *current != '_')
		{
			trackedLength = 0;
			return false;
		}

		current++;
		while (current != end)
		{
			char c = *current;
			if (!IsLetterOrDigit(c) && c != '_')
			{
				break;
			}
			current++;
		}

		trackedLength = (size_t)(current - (text + offset));
		return true;
	}
}
#endif