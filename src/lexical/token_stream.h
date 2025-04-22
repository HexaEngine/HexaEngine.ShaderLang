#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include "lexical/lexer.h"
#include "log.h"

#include <string>
#include <stack>
#include <deque>

namespace HXSL
{
	using namespace HXSL::Lexer;

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
	private:
		struct TokenStreamState
		{
			LexerState state;
			Token lastToken;
			Token currentToken;
			size_t tokenPosition;

			TokenStreamState() = default;

			TokenStreamState(LexerState lexerState) : state(lexerState), lastToken({}), currentToken({}), tokenPosition(0)
			{
			}

			bool IsEndOfTokens() const { return state.IsEOF(); }
		};

		TokenStreamState streamState;
		LexerConfig* config;
		std::stack<TokenStreamState> stack;
		size_t currentStack;
		TokenCache cache;

	public:
		TokenStream(LexerState lexerState, LexerConfig* config) : streamState(TokenStreamState(lexerState)), config(config), currentStack(0)
		{
		}

		template<typename... Args>
		void LogFormatted(LogLevel level, const std::string& message, Args&&... args) const
		{
			streamState.state.GetLogger()->LogFormatted(level, message, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogError(const std::string& message, Args&&... args) const
		{
			LogFormatted(LogLevel_Error, message, std::forward<Args>(args)...);
		}

		bool HasErrors() const noexcept { return streamState.state.HasCriticalErrors(); }

		Token LastToken() const { return streamState.lastToken; }

		Token Current() const { return streamState.currentToken; }

		bool IsEndOfTokens() const { return streamState.IsEndOfTokens(); }

		TextSpan MakeFromLast(const TextSpan& span) const
		{
			return span.merge(streamState.lastToken.Span);
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
			current = streamState.currentToken;
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

		bool TryGetKeyword(Keyword keyword)
		{
			return TryGetTypeValue(TokenType_Keyword, keyword);
		}

		bool TryGetKeywords(std::unordered_set<Keyword> keywords)
		{
			Token current;
			if (!TryGetToken(TokenType_Keyword, current))
			{
				return false;
			}

			Keyword keyword = static_cast<Keyword>(current.Value);
			if (keywords.find(keyword) != keywords.end())
			{
				TryAdvance();
				return true;
			}
			return false;
		}

		bool TryGetOperator(Operator op)
		{
			return TryGetTypeValue(TokenType_Operator, op);
		}

		bool TryGetAnyOperator(Operator& op)
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

		bool TryGetNumber(Number& number)
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
				streamState.state.LogErrorFormatted("Unexpected token, expected an '%s'", ToString(type).c_str());
				return false;
			}
			if (advance)
			{
				Advance();
			}
			return true;
		}

		bool ExpectOperator(Operator op)
		{
			Token token;
			auto result = Expect(TokenType_Operator, token);
			return result && op == static_cast<Operator>(token.Value);
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

		bool ExpectKeywords(const std::unordered_set<Keyword>& keywords, Keyword& keyword)
		{
			Token token;
			auto result = Expect(TokenType_Keyword, token, false);

			keyword = static_cast<Keyword>(token.Value);
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
				streamState.state.LogErrorFormatted("Unexpected delimiter, expected an '%c'", delimiter);
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
				streamState.state.LogErrorFormatted("Unexpected delimiter, found an '%c' when it was not expected.", delimiter);
				return false;
			}

			return true;
		}
	};
}
#endif