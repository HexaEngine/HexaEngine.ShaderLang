#include "lexer.hpp"
#include "utils/text_helper.hpp"
#include "numbers.hpp"
#include "pch/localization.hpp"

namespace HXSL
{
	LexerState LexerContext::MakeState()
	{
		return LexerState(this);
	}

	static Token TokenizeStepInner(LexerState& state, const LexerConfig* config)
	{
		const char* pCurrent = state.Current();
		char current = *pCurrent;
		size_t i = state.Index;
		size_t len = state.GetLength();

		int keyword;
		size_t keywordLength;
		if (config->keywords.Find(state.AsSpan(), keyword, keywordLength) && keyword != 0)
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
		if (config->operators.Find(state.AsSpan(), op, operatorLength))
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
			ParseNumber(state.GetBuffer(), len, i, isHex, isBinary, false, num, numberLength);
			state.IndexNext += numberLength;
			return Token(state.AsTextSpan(i, numberLength), TokenType_Numeric, num);
		}

		if (config->delimiters.find(current) != config->delimiters.end())
		{
			state.IndexNext += 1;
			return Token(state.AsTextSpan(i, 1), TokenType_Delimiter, current);
		}

		if (state.MatchPair(current, '/', '/'))
		{
			size_t lineCommentLen = state.FindEndOfLine(i + 2);
			state.IndexNext += lineCommentLen + 2;

			auto token = Token(state.AsTextSpan(i, lineCommentLen), TokenType_Comment);
			state.NewLine();
			return token;
		}

		if (state.MatchPair(current, '/', '*'))
		{
			size_t lines = 0;
			size_t trackedLength = 0;
			if (!state.LookAhead(i + 2, "*/", trackedLength, lines))
			{
				state.LogFormatted(MISSING_END_COMMENT);
				return {};
			}
			state.IndexNext += trackedLength + 4;

			auto token = Token(state.AsTextSpan(i, trackedLength + 3), TokenType_Comment);
			state.NewLine(static_cast<uint32_t>(lines));
			return token;
		}

		if (current == '"')
		{
			size_t lines = 0;
			size_t trackedLength = 0;
			if (!state.LookAhead(i + 1, '"', trackedLength, lines))
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
			return Token(state.AsTextSpan(i, identifierLength), TokenType_Identifier);
		}

		state.LogFormatted(INVALID_TOKEN);
		state.IndexNext++;
		return {};
	}

	Token Lexer::TokenizeStep(LexerState& state)
	{
		size_t i = state.Index;
		if (i >= state.GetLength())
		{
			state.IndexNext++;
			return {};
		}

		auto config = state.GetConfig();
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
						size_t idx = current - state.GetBuffer();
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
						size_t idx = current - state.GetBuffer();
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

			state.Jump(current - state.GetBuffer());
		}

		if (state.IsEOF())
		{
			return {};
		}

		return TokenizeStepInner(state, config);
	}
}