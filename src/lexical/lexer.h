#ifndef LEXER_H
#define LEXER_H

#include <stdexcept>
#include <unordered_set>
#include "text_helper.h"
#include "tst.hpp"
#include "log.h"
#include "token.h"
#include "numbers.h"
#include "lang/language.h"

namespace HXSL
{
	struct LexerState
	{
	private:
		ILogger* Logger;

	public:
		char* Text;
		size_t Length;
		size_t Index;
		size_t IndexNext;
		size_t Line;
		size_t Column;
		bool TreatIdentiferAsLiteral;

		LexerState()
			: Logger(nullptr), Text(nullptr), Length(0), Index(0), IndexNext(0), Line(1), Column(1), TreatIdentiferAsLiteral(false) {
		}

		LexerState(ILogger* logger, char* text, size_t length)
			: Logger(logger), Text(text), Length(length), Index(0), IndexNext(0), Line(1), Column(1), TreatIdentiferAsLiteral(false) {
		}

		ILogger* logger() const noexcept { return Logger; }

		char* Current() const noexcept
		{
			return Text + Index;
		}

		bool IsEOF() const noexcept
		{
			return Index >= Length;
		}

		bool HasCriticalErrors() const noexcept
		{
			return Logger->HasCriticalErrors();
		}

		void Advance()
		{
			size_t diff = IndexNext - Index;
			Index = IndexNext;
			Column += diff;
		}

		void SkipWhitespace()
		{
			size_t start = Index;
			TextHelper::SkipLeadingWhitespace(Text, Index, Length);
			size_t skipped = Index - start;
			Column += skipped;
			IndexNext = Index;
		}

		void SkipNewLines()
		{
			char* start = Text + Index;
			char* current = start;
			char* end = Text + Length;
			char c;
			while (current != end)
			{
				c = *current;
				bool isLF = c == '\n';
				if (!isLF && c != '\r')
				{
					break;
				}

				int width = 1;
				char* next = current + 1;
				if (!isLF && next < end && *next == '\n')
				{
					width++;
				}

				NewLine();
				current = current + width;
			}

			size_t skipped = current - start;
			Index += skipped;
			IndexNext = Index;
		}

		void Jump(size_t index)
		{
			HXSL_ASSERT(Index < index, "Jump index must be larger than the current.");
			IndexNext = Index = index;
		}

		void NewLine()
		{
			Line++;
			Column = 1;
		}

		bool MatchPair(char current, char first, char second) const
		{
			return TextHelper::MatchPair(Text, Index, Length, current, first, second);
		}

		size_t FindEndOfLine(size_t start) const
		{
			return TextHelper::FindEndOfLine(Text, start, Length);
		}

		size_t FindWordBoundary(size_t start) const
		{
			return TextHelper::FindWordBoundary(Text, start, Length);
		}

		size_t FindOperatorBoundary(size_t start, std::unordered_set<char> delimiters) const
		{
			return TextHelper::FindOperatorBoundary(Text, start, Length, delimiters);
		}

		size_t FindWhitespaceWordBoundary(size_t start) const
		{
			return TextHelper::FindWhitespaceWordBoundary(Text, start, Length);
		}

		bool LookAhead(size_t start, char target, size_t& trackedLength) const
		{
			return TextHelper::LookAhead(Text, start, Length, target, trackedLength);
		}

		bool LookAhead(size_t start, const std::string& target, size_t& trackedLength) const
		{
			return TextHelper::LookAhead(Text, start, Length, target, trackedLength);
		}

		bool StartsWith(const std::string& value) const
		{
			return TextHelper::StartsWith(Text, Index, Length, value);
		}

		bool TryParseIdentifier(size_t& trackedLength) const
		{
			return TextHelper::TryParseIdentifier(Text, Index, Length, trackedLength);
		}

		TextSpan AsTextSpan(size_t start, size_t length) const
		{
			return TextSpan(Text, start, length, Line, Column);
		}

		TextSpan AsTextSpan() const
		{
			return TextSpan(Text, Index, Length - Index, Line, Column);
		}

		void LogError(const std::string& message)
		{
			Logger->LogFormatted(HXSLLogLevel_Error, "%s (Line: %i, Column: %i)", message.c_str(), Line, Column);
		}

		template <typename... Args>
		void LogErrorFormatted(const std::string& format, Args&&... args)
		{
			std::string formatted_message = format + " (Line: %i, Column: %i)";
			Logger->LogFormatted(HXSLLogLevel_Error, formatted_message, std::forward<Args>(args)..., Line, Column);
		}
	};

	class LexerConfig
	{
	public:
		bool EnableNewline;
		bool EnableWhitespace;
		bool SpecialParseTreatIdentiferAsLiteral;
		bool EnableCodeblock;
		std::unordered_set<char> Delimiters;
		TernarySearchTreeDictionary<int> Keywords;
		TernarySearchTreeDictionary<int> Operators;

		LexerConfig() : EnableNewline(false), EnableWhitespace(false), SpecialParseTreatIdentiferAsLiteral(false), EnableCodeblock(false)
		{
		}

		LexerConfig(bool enableNewline, bool enableWhitespace, bool specialParseTreatIdentiferAsLiteral, bool enableCodeblock, const std::unordered_set<char>& delimiters, const TernarySearchTreeDictionary<int>& keywords, const TernarySearchTreeDictionary<int>& operators)
			: EnableNewline(enableNewline), EnableWhitespace(enableWhitespace), SpecialParseTreatIdentiferAsLiteral(specialParseTreatIdentiferAsLiteral), EnableCodeblock(enableCodeblock), Delimiters(delimiters), Keywords(keywords), Operators(operators)
		{
		}
	};

	class LexerConfigHXSL
	{
	public:
		static LexerConfig* Instance()
		{
			static LexerConfig* instance;
			static bool initialized = false;
			if (!initialized)
			{
				instance = new LexerConfig();
				BuildKeywordTST(&instance->Keywords);
				BuildOperatorTST(&instance->Operators);
				instance->Delimiters = { '{', '}', '[', ']', '(', ')', ',', ':', '.', ';', '#', '@' };
				initialized = true;
			}
			return instance;
		}

	private:
		LexerConfigHXSL() = delete;
		~LexerConfigHXSL() = delete;
		LexerConfigHXSL(const LexerConfigHXSL&) = delete;
		LexerConfigHXSL& operator=(const LexerConfigHXSL&) = delete;
	};
}

namespace Lexer
{
	using namespace HXSL;
	Token TokenizeStep(LexerState& state, LexerConfig* config);
}

#endif