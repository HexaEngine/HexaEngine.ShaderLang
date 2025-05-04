#ifndef LEXER_H
#define LEXER_H

#include <stdexcept>
#include <unordered_set>
#include "utils/text_helper.h"
#include "utils/tst.hpp"
#include "io/logger.hpp"
#include "token.h"
#include "numbers.h"
#include "lang/language.h"
#include "diagnostic_code.hpp"
#include "lexer_input_stream.hpp"

namespace HXSL
{
	namespace Lexer
	{
		struct LexerState
		{
		private:
			ILogger* logger;

		public:
			SourceFile* source;
			const char* Text;
			size_t Length;
			size_t Index;
			size_t IndexNext;
			uint32_t Line;
			uint32_t Column;
			bool TreatIdentiferAsLiteral;

			LexerState()
				: logger(nullptr), source(nullptr), Text(nullptr), Length(0), Index(0), IndexNext(0), Line(1), Column(1), TreatIdentiferAsLiteral(false) {
			}

			LexerState(ILogger* logger, SourceFile* source, const char* text, size_t length)
				: logger(logger), source(source), Text(text), Length(length), Index(0), IndexNext(0), Line(1), Column(1), TreatIdentiferAsLiteral(false) {
			}

			ILogger* GetLogger() const noexcept { return logger; }

			const char* Current() const noexcept
			{
				return Text + Index;
			}

			const char* End() const noexcept
			{
				return Text + Length;
			}

			bool IsEOF() const noexcept
			{
				return Index >= Length;
			}

			bool HasCriticalErrors() const noexcept
			{
				return logger->HasCriticalErrors();
			}

			void Advance()
			{
				size_t diff = IndexNext - Index;
				Index = IndexNext;
				Column += static_cast<uint32_t>(diff);
			}

			void SkipWhitespace()
			{
				size_t start = Index;
				TextHelper::SkipLeadingWhitespace(Text, Index, Length);
				size_t skipped = Index - start;
				Column += static_cast<uint32_t>(skipped);
				IndexNext = Index;
			}

			void SkipNewLines()
			{
				const char* start = Text + Index;
				const char* current = start;
				const char* end = Text + Length;
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
					const char* next = current + 1;
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

			size_t FindOperatorBoundary(size_t start, const std::unordered_set<char>& delimiters) const
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
				return TextSpan(source, start, length, Line, Column);
			}

			TextSpan AsTextSpan() const
			{
				return TextSpan(source, Index, Length - Index, Line, Column);
			}

			StringSpan AsSpan() const
			{
				return StringSpan(Text, Index, Length - Index);
			}

			template <typename... Args>
			void LogFormatted(DiagnosticCode code, Args&&... args) const
			{
				logger->LogFormattedEx(code, " (Line: {}, Column: {})", std::forward<Args>(args)..., Line, Column);
			}
		};

		class LexerConfig
		{
		public:
			bool enableNewline;
			bool enableWhitespace;
			bool specialParseTreatIdentiferAsLiteral;
			std::unordered_set<char> delimiters;
			TernarySearchTreeDictionary<int> keywords;
			TernarySearchTreeDictionary<int> operators;

			LexerConfig() : enableNewline(false), enableWhitespace(false), specialParseTreatIdentiferAsLiteral(false)
			{
			}

			LexerConfig(bool enableNewline, bool enableWhitespace, bool specialParseTreatIdentiferAsLiteral, const std::unordered_set<char>& delimiters, const TernarySearchTreeDictionary<int>& keywords, const TernarySearchTreeDictionary<int>& operators)
				: enableNewline(enableNewline), enableWhitespace(enableWhitespace), specialParseTreatIdentiferAsLiteral(specialParseTreatIdentiferAsLiteral), delimiters(delimiters), keywords(keywords), operators(operators)
			{
			}
		};

		class HXSLLexerConfig
		{
		public:
			static LexerConfig* Instance()
			{
				static LexerConfig* instance;
				static bool initialized = false;
				if (!initialized)
				{
					instance = new LexerConfig();
					instance->enableNewline = true;
					BuildKeywordTST(&instance->keywords);
					BuildOperatorTST(&instance->operators);
					instance->delimiters = { '{', '}', '[', ']', '(', ')', ',', ';', '#', '@' };
					initialized = true;
				}
				return instance;
			}

		private:
			HXSLLexerConfig() = delete;
			~HXSLLexerConfig() = delete;
			HXSLLexerConfig(const HXSLLexerConfig&) = delete;
			HXSLLexerConfig& operator=(const HXSLLexerConfig&) = delete;
		};

		Token TokenizeStep(LexerState& state, LexerConfig* config);
	}
}

#endif