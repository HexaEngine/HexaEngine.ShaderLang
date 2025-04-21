#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include "lexical/lexer.h"
#include "log.h"

#include <string>
#include <stack>

namespace HXSL
{
	struct TokenCache
	{
		std::deque<Token> tokens;
		size_t maxSize = 10;
		size_t tokenPosition = 0;

		void IncrementPosition()
		{
			tokenPosition++;
		}

		void AddToken(const Token& token)
		{
			if (tokens.size() >= maxSize)
			{
				tokens.pop_front();
				tokenPosition++;
			}
			tokens.push_back(token);
		}

		Token PeekToken(size_t index) const
		{
			index -= tokenPosition;
			if (index < tokens.size())
			{
				return tokens[index];
			}
			return {};
		}

		Token GetToken()
		{
			if (!tokens.empty())
			{
				Token token = tokens.front();
				tokens.pop_front();
				tokenPosition++;
				return token;
			}
			return {};
		}

		size_t Position() const
		{
			return tokenPosition;
		}

		size_t EndPosition() const
		{
			return tokenPosition + tokens.size();
		}

		bool CanAccess(size_t pos) const
		{
			return tokenPosition >= pos && pos < EndPosition();
		}

		size_t Size() const
		{
			return tokens.size();
		}

		bool IsEmpty() const
		{
			return tokens.empty();
		}

		void Flush(size_t pos)
		{
			tokens.clear();
			tokenPosition = pos;
		}
	};

	struct TokenStream
	{
		struct TokenStreamState
		{
			LexerState State;
			Token LastToken;
			Token CurrentToken;
			size_t TokenPosition;

			TokenStreamState() = default;

			TokenStreamState(LexerState lexerState) : State(lexerState), LastToken({}), CurrentToken({}), TokenPosition(0)
			{
			}

			bool IsEndOfTokens() const { return State.IsEOF(); }
		};

		TokenStreamState StreamState;
		LexerConfig* Config;
		std::stack<TokenStreamState> stack;
		size_t currentStack;
		TokenCache Cache;

		TokenStream(LexerState lexerState, LexerConfig* config) : StreamState(TokenStreamState(lexerState)), Config(config), currentStack(0)
		{
		}

		bool HasErrors() const noexcept { return StreamState.State.HasCriticalErrors(); }

		Token LastToken() const { return StreamState.LastToken; }

		Token Current() const { return StreamState.CurrentToken; }

		bool IsEndOfTokens() const { return StreamState.IsEndOfTokens(); }

		TextSpan MakeFromLast(const TextSpan& span) const
		{
			return span.merge(StreamState.LastToken.Span);
		}

		TextSpan MakeFromLast(const Token& token) const
		{
			return MakeFromLast(token.Span);
		}

		void PushState();

		void PopState(bool restore = true);

		void Advance();

		bool TryAdvance();

		bool TryGetToken(TokenType type, Token& current) const
		{
			current = StreamState.CurrentToken;
			return current.Type == type;
		}

		bool TryGetTypeValue(TokenType type, int value)
		{
			Token current;
			if (!TryGetToken(type, current))
			{
				return false;
			}
			if (current.Value == value)
			{
				TryAdvance();
				return true;
			}
			return false;
		}

		bool TryGetTypeAnyValue(TokenType type, int& value)
		{
			Token current;
			if (!TryGetToken(type, current))
			{
				return false;
			}

			current.Value = value;
			TryAdvance();
			return true;
		}

		bool TryGetDelimiter(char delimiter)
		{
			Token current;
			if (!TryGetToken(TokenType_Delimiter, current))
			{
				return false;
			}
			if (current.Span[0] == delimiter)
			{
				TryAdvance();
				return true;
			}
			return false;
		}

		bool TryGetKeyword(HXSLKeyword keyword)
		{
			return TryGetTypeValue(TokenType_Keyword, keyword);
		}

		bool TryGetKeywords(std::unordered_set<HXSLKeyword> keywords)
		{
			Token current;
			if (!TryGetToken(TokenType_Keyword, current))
			{
				return false;
			}

			HXSLKeyword keyword = static_cast<HXSLKeyword>(current.Value);
			if (keywords.find(keyword) != keywords.end())
			{
				TryAdvance();
				return true;
			}
			return false;
		}

		bool TryGetOperator(HXSLOperator op)
		{
			return TryGetTypeValue(TokenType_Operator, op);
		}

		bool TryGetAnyOperator(HXSLOperator& op)
		{
			if (Current().isOperator(op))
			{
				TryAdvance();
				return true;
			}
			return false;
		}

		bool TryGetIdentifier(TextSpan& span)
		{
			Token current;
			if (TryGetToken(TokenType_Identifier, current))
			{
				TryAdvance();
				span = current.Span;
				return true;
			}

			span = {};
			return false;
		}

		bool TryGetLiteral(TextSpan& span)
		{
			Token current;
			if (TryGetToken(TokenType_Literal, current))
			{
				TryAdvance();
				span = current.Span;
				return true;
			}

			span = {};
			return false;
		}

		bool TryGetNumber(HXSLNumber& number)
		{
			Token current;
			if (TryGetToken(TokenType_Numeric, current))
			{
				TryAdvance();
				number = current.Numeric;
				return true;
			}

			number = {};
			return false;
		}

		bool Expect(TokenType type, Token& current, bool advance = true)
		{
			current = Current();
			if (current.Type != type)
			{
				StreamState.State.LogErrorFormatted("Unexpected token, expected an '%s'", ToString(type).c_str());
				return false;
			}
			if (advance)
			{
				Advance();
			}
			return true;
		}

		bool ExpectOperator(HXSLOperator op)
		{
			Token token;
			auto result = Expect(TokenType_Operator, token);
			return result && op == static_cast<HXSLOperator>(token.Value);
		}

		bool ExpectLiteral(TextSpan& literal)
		{
			Token token;
			auto result = Expect(TokenType_Literal, token);
			literal = token.Span;
		}

		bool ExpectIdentifier(TextSpan& identifier)
		{
			Token token;
			auto result = Expect(TokenType_Identifier, token);
			identifier = token.Span;
			return result;
		}

		bool ExpectCodeblock(TextSpan& literal)
		{
			Token token;
			auto result = Expect(TokenType_Codeblock, token);
			literal = token.Span;
		}

		bool ExpectKeywords(const std::unordered_set<HXSLKeyword>& keywords, HXSLKeyword& keyword)
		{
			Token token;
			auto result = Expect(TokenType_Keyword, token, false);

			keyword = static_cast<HXSLKeyword>(token.Value);
			if (result && keywords.find(keyword) != keywords.end())
			{
				Advance();
				return true;
			}
			return false;
		}

		bool ExpectDelimiter(char delimiter, Token& token)
		{
			auto result = Expect(TokenType_Delimiter, token, false);
			if (!result || token.Span[0] != delimiter)
			{
				StreamState.State.LogErrorFormatted("Unexpected delimiter, expected an '%c'", delimiter);
				return false;
			}
			Advance();
			return result;
		}

		bool ExpectDelimiter(char delimiter)
		{
			Token token;
			return ExpectDelimiter(delimiter, token);
		}

		bool ExpectNoDelimiters(std::unordered_set<char> delimiters)
		{
			Token current = Current();
			if (current.Type != TokenType_Delimiter) return true;
			char delimiter = current.Span[0];
			if (delimiters.find(delimiter) != delimiters.end())
			{
				StreamState.State.LogErrorFormatted("Unexpected delimiter, found an '%c' when it was not expected.", delimiter);
				return false;
			}

			return true;
		}
	};
}
#endif