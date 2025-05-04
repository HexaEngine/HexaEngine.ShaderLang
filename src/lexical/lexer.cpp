#include "lexer.h"
#include "utils/text_helper.h"
#include "utils/tst.hpp"
#include "io/logger.hpp"
#include "numbers.h"
#include "generated/localization.hpp"

#include <stdexcept>
#include <cctype>

namespace HXSL
{
	namespace Lexer
	{
		static Token TokenizeStepInner(LexerState& state, LexerConfig* config)
		{
			const char* pCurrent = state.Current();
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
				return Token(state.AsTextSpan(i, 1), TokenType_Delimiter, current);
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
					state.LogFormatted(MISSING_END_COMMENT);
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
					state.LogFormatted(MISSING_QUOTE);
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

			state.LogFormatted(INVALID_TOKEN);
			return {};
		}

		static bool isspace(char32_t c)
		{
			if (c <= 0x20) {
				return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
			}

			return (c >= 0x2000 && c <= 0x200A) || // Range for common space characters
				(c == 0x202F) ||                // Narrow no-break space
				(c == 0x3000) ||                // Ideographic space
				(c == 0x205F);                  // Medium Mathematical Space
		}

		static char32_t decodeuni(const char* text, size_t& width, size_t max_len)
		{
			if (!text || max_len == 0)
			{
				width = 0;
				return 0xFFFD;
			}

			uint8_t c1 = static_cast<uint8_t>(text[0]);

			if (c1 <= 0x7F)
			{
				width = 1;
				return c1;
			}

			if ((c1 >> 5) == 0x6)
			{
				if (max_len < 2)
				{
					width = 1;
					return 0xFFFD;
				}

				width = 2;
				return ((c1 & 0x1F) << 6) | (static_cast<uint8_t>(text[1]) & 0x3F);
			}

			if ((c1 >> 4) == 0xE)
			{
				if (max_len < 3)
				{
					width = 1;
					return 0xFFFD;
				}

				width = 3;
				return ((c1 & 0x0F) << 12) | ((static_cast<uint8_t>(text[1]) & 0x3F) << 6) | (static_cast<uint8_t>(text[2]) & 0x3F);
			}

			if ((c1 >> 3) == 0x1E)
			{
				if (max_len < 4)
				{
					width = 1;
					return 0xFFFD;
				}

				width = 4;
				return ((c1 & 0x07) << 18) | ((static_cast<uint8_t>(text[1]) & 0x3F) << 12) | ((static_cast<uint8_t>(text[2]) & 0x3F) << 6) | (static_cast<uint8_t>(text[3]) & 0x3F);
			}

			width = 1;
			return 0xFFFD;
		}

		Token TokenizeStep(LexerState& state, LexerConfig* config)
		{
			size_t i = state.Index;
			if (i >= state.Length)
			{
				state.IndexNext++;
				return {};
			}

			const char* current = state.Current();
			if (std::isspace(*current))
			{
				auto end = state.End();
				while (current < end)
				{
					char c = *current;
					bool isCR = c == '\r';
					if (isCR || c == '\n')
					{
						size_t width = 1;
						const char* next = current + 1;
						if (isCR && next < end && *next == '\n')
						{
							width++;
						}

						state.NewLine();

						if (config->enableNewline)
						{
							size_t idx = current - state.Text;
							state.IndexNext = idx + width;
							return Token(state.AsTextSpan(idx, width), TokenType_NewLine);
						}

						current = current + width;
					}
					else if (std::isspace(c))
					{
						const char* next = current + 1;
						while (next < end)
						{
							c = *next;
							if (!std::isspace(c) || c == '\n' || c == '\r')
							{
								break;
							}
							next++;
						}
						size_t width = next - current;
						if (config->enableWhitespace)
						{
							size_t idx = current - state.Text;
							state.IndexNext = idx + width;
							return Token(state.AsTextSpan(idx, width), TokenType_Whitespace);
						}

						current = next;
					}
					else
					{
						break;
					}
				}

				state.Jump(current - state.Text);
			}

			if (state.IsEOF())
			{
				return {};
			}

			return TokenizeStepInner(state, config);
		}
	}
}