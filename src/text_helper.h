#ifndef TEXT_HELPER_H
#define TEXT_HELPER_H
#include <cctype>
#include <unordered_set>
#include <string>

namespace TextHelper
{
	bool IsLetterOrDigit(char c);

	bool MatchPair(const char* text, size_t index, size_t length, char current, char first, char second);

	size_t FindEnd(size_t start, const char* text, size_t length, bool(*selector)(char));

	size_t FindEndOfLine(const char* text, size_t offset, size_t length);

	size_t FindWordBoundary(const char* text, size_t offset, size_t length);

	size_t FindOperatorBoundary(const char* text, size_t offset, size_t length, std::unordered_set<char> delimiter);

	size_t FindWhitespaceWordBoundary(const char* text, size_t offset, size_t length);

	bool LookAhead(const char* text, size_t offset, size_t length, char target, size_t& trackedLength);

	bool LookAhead(const char* text, size_t offset, size_t length, const std::string& target, size_t& trackedLength);

	bool StartsWith(const char* text, size_t offset, size_t length, const std::string& value);

	size_t CountLeadingWhitespace(const char* text, size_t length);

	size_t CountLeadingWhitespace(const char* text);

	size_t CountTrailingWhitespace(const char* text, size_t length);

	void SkipLeadingWhitespace(const char* text, size_t& index, size_t length);

	void SkipLeadingWhitespace(const char*& text, size_t length);

	void SkipTrailingWhitespace(const char* text, size_t index, size_t& length);

	void SkipTrailingWhitespace(const char* text, size_t& length);

	bool TryParseIdentifier(const char* text, size_t offset, size_t length, size_t& trackedLength);
}
#endif