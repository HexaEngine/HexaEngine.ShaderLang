#include "lexer.h"
#include "text_helper.h"
#include "tst.hpp"
#include "log.h"
#include "numbers.h"

#include <stdexcept>
#include <cctype>

namespace HXSL
{
	namespace Lexer
	{
		static Token TokenizeStepInner(LexerState& state, LexerConfig* config)
		{
			char* pCurrent = state.Current();
			char current = *pCurrent;
			size_t i = state.Index;
			size_t len = state.Length;

			int keyword;
			size_t keywordLength;
			if (config->keywords.MatchLongestPrefix(state.AsTextSpan(), keyword, keywordLength) && keyword != 0)
			{
				size_t wordLength = state.FindWordBoundary(i + keywordLength);
				if (wordLength == 0)
				{
					state.IndexNext += keywordLength;
					return Token(state.AsTextSpan(i, keywordLength), TokenType_Keyword, keyword);
				}
			}

			int op;
			size_t operatorLength;
			if (config->operators.MatchLongestPrefix(state.AsTextSpan(), op, operatorLength))
			{
				// causes problems with -( for example, that's why we forward the delimiters.
				size_t wordLength = state.FindOperatorBoundary(i + operatorLength, config->delimiters);
				if (wordLength == 0)
				{
					state.IndexNext += operatorLength;
					return Token(state.AsTextSpan(i, operatorLength), TokenType_Operator, op);
				}
			}

			bool isHex = current == '0' && i + 1 < len && (pCurrent[1] == 'x' || pCurrent[1] == 'X');
			bool isBinary = current == '0' && i + 1 < len && (pCurrent[1] == 'b' || pCurrent[1] == 'B');
			if (std::isdigit(current) || (current == '.' && i + 1 < len && std::isdigit(pCurrent[1])) || isHex || isBinary)
			{
				size_t numberLength;
				Number num;
				ParseNumber(state.Text, len, i, isHex, isBinary, false, num, numberLength);
				state.IndexNext += numberLength;
				return Token(state.AsTextSpan(i, numberLength), TokenType_Numeric, num);
			}

			if (config->delimiters.find(current) != config->delimiters.end())
			{
				if (current == ':')
				{
					state.TreatIdentiferAsLiteral = config->specialParseTreatIdentiferAsLiteral;
				}
				else
				{
					state.TreatIdentiferAsLiteral = false;
				}
				state.IndexNext += 1;
				return Token(state.AsTextSpan(i, 1), TokenType_Delimiter);
			}

			if (config->enableCodeblock && state.MatchPair(current, '<', '!'))
			{
				size_t trackedLength;
				if (!state.LookAhead(i + 2, "!>", trackedLength))
				{
					state.LogError("Inbalanced code block.");
					return {};
				}
				state.IndexNext += trackedLength + 4;
				return Token(state.AsTextSpan(i + 2, trackedLength - 1), TokenType_Codeblock);
			}

			if (state.MatchPair(current, '/', '/'))
			{
				size_t lineCommentLen = state.FindEndOfLine(i + 2);
				state.IndexNext += lineCommentLen + 2;
				state.NewLine();
				return Token(state.AsTextSpan(i, lineCommentLen), TokenType_Comment);
			}

			if (state.MatchPair(current, '/', '*'))
			{
				size_t trackedLength;
				if (!state.LookAhead(i + 2, "*/", trackedLength))
				{
					state.LogError("Inbalanced comment block.");
					return {};
				}
				state.IndexNext += trackedLength + 4;
				return Token(state.AsTextSpan(i, trackedLength + 3), TokenType_Comment);
			}

			if (current == '"')
			{
				size_t trackedLength;
				if (!state.LookAhead(i + 1, '"', trackedLength))
				{
					state.LogError("Inbalanced literal.");
					return {};
				}
				state.IndexNext += trackedLength + 2;
				return Token(state.AsTextSpan(i + 1, trackedLength), TokenType_Literal);
			}

			size_t identifierLength;
			if (state.TryParseIdentifier(identifierLength))
			{
				state.IndexNext += identifierLength;
				return Token(state.AsTextSpan(i, identifierLength), state.TreatIdentiferAsLiteral ? TokenType_Literal : TokenType_Identifier);
			}

			state.LogError("Unknown token.");
			return {};
		}

		Token TokenizeStep(LexerState& state, LexerConfig* config)
		{
			char* pCurrent = state.Current();
			char current = *pCurrent;
			size_t i = state.Index;

			if (config->enableNewline && (current == '\n' || current == '\r'))
			{
				int width = 1;
				char* next = pCurrent + 1;
				if (i + 1 < state.Length && *next == '\n')
				{
					width++;
				}

				state.NewLine();

				state.IndexNext = i + width;
				return Token(state.AsTextSpan(i, width), TokenType_NewLine);
			}
			else
			{
				state.SkipNewLines();
			}

			if (config->enableWhitespace && std::isspace(current))
			{
				size_t length = TextHelper::CountLeadingWhitespace(pCurrent + 1) + 1;
				state.IndexNext = i + length;
				return Token(state.AsTextSpan(i, length), TokenType_Whitespace);
			}
			else
			{
				state.SkipWhitespace();
			}

			if (state.IsEOF())
			{
				return {};
			}

			return TokenizeStepInner(state, config);
		}
	}
}