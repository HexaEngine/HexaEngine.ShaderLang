#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include "lexical/lexer.h"
#include "io/log.h"

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
			size_t tokenPosition = 0;

			TokenStreamState() = default;

			TokenStreamState(LexerState lexerState) : state(lexerState), lastToken({}), currentToken({}), tokenPosition(0)
			{
			}

			bool IsEndOfTokens() const { return state.IsEOF(); }
		};

		TokenStreamState streamState;
		TokenStreamState restorePoint;
		LexerConfig* config;
		std::stack<TokenStreamState> stack;
		size_t currentStack;
		TokenCache cache;

	public:
		TokenStream() = default;
		TokenStream(LexerState lexerState, LexerConfig* config) : streamState(TokenStreamState(lexerState)), config(config), currentStack(0)
		{
		}

		template<typename... Args>
		void LogFormatted(DiagnosticCode code, Args&&... args) const
		{
			streamState.state.LogFormatted(code, std::forward<Args>(args)...);
		}

		bool HasCriticalErrors() const noexcept { return streamState.state.HasCriticalErrors(); }

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

		void MakeRestorePoint()
		{
			restorePoint = streamState;
		}

		void RestoreFromPoint()
		{
			streamState = restorePoint;
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

		bool TryGetKeywords(const std::unordered_set<Keyword>& keywords)
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

		bool TryGetUnaryOperator(Operator& op)
		{
			if (Current().isUnaryOperator(op))
			{
				TryAdvance();
				return true;
			}
			return false;
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
				return false;
			}
			if (advance)
			{
				Advance();
			}
			return true;
		}

		template <typename... Args>
		bool ExpectOperator(Operator op, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Operator, token);
			if (!result || op != static_cast<Operator>(token.Value))
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			return true;
		}

		template <typename... Args>
		bool ExpectAnyOperator(Operator& op, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Operator, token);
			if (!result)
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			op = static_cast<Operator>(token.Value);
			return true;
		}

		template <typename... Args>
		bool ExpectLiteral(TextSpan& literal, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Literal, token);
			if (!result)
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			literal = token.Span;
			return true;
		}

		template <typename... Args>
		bool ExpectIdentifier(TextSpan& identifier, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Identifier, token);
			if (!result)
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			identifier = token.Span;
			return true;
		}

		template <typename... Args>
		bool ExpectKeyword(const Keyword& keyword, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Keyword, token, false);
			if (!result || keyword != static_cast<Keyword>(token.Value))
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			Advance();
			return true;
		}

		template <typename... Args>
		bool ExpectKeywords(const std::unordered_set<Keyword>& keywords, Keyword& keyword, DiagnosticCode code, Args&&... args)
		{
			Token token;
			auto result = Expect(TokenType_Keyword, token, false);
			keyword = static_cast<Keyword>(token.Value);
			if (!result || keywords.find(keyword) == keywords.end())
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			Advance();
			return true;
		}

		template <typename... Args>
		bool ExpectDelimiter(char delimiter, Token& token, DiagnosticCode code, Args&&... args)
		{
			auto result = Expect(TokenType_Delimiter, token, false);
			if (!result || token.Span[0] != delimiter)
			{
				streamState.state.LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			TryAdvance();
			return true;
		}

		template <typename... Args>
		bool ExpectDelimiter(char delimiter, DiagnosticCode code, Args&&... args)
		{
			Token token;
			return ExpectDelimiter(delimiter, token, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectNumeric(Number& numberOut, Token& token, DiagnosticCode code, Args&&... args)
		{
			auto result = Expect(TokenType_Numeric, token, false);
			if (!result)
			{
				streamState.state.LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}
			numberOut = token.Numeric;
			TryAdvance();
			return true;
		}

		template <typename... Args>
		bool ExpectNumeric(Number& numberOut, DiagnosticCode code, Args&&... args)
		{
			Token token;
			return ExpectNumeric(numberOut, token, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectNoDelimiters(const std::unordered_set<char>& delimiters, DiagnosticCode code, Args&&... args)
		{
			Token current = Current();
			if (current.Type != TokenType_Delimiter) return true;
			char delimiter = current.Span[0];
			if (delimiters.find(delimiter) != delimiters.end())
			{
				LogFormatted(code, std::forward<Args>(args)...);
				return false;
			}

			return true;
		}
	};

	struct TokenStreamAdapter
	{
	private:
		TokenStream& tokenStream;

	public:
		TokenStreamAdapter(TokenStream& ts) : tokenStream(ts) {}

		TokenStream& GetTokenStream() const
		{
			return tokenStream;
		}

		bool IsEndOfTokens() const { return tokenStream.IsEndOfTokens(); }
		Token LastToken() const { return tokenStream.LastToken(); }
		Token Current() const { return tokenStream.Current(); }

		TextSpan MakeFromLast(const TextSpan& span) const { return tokenStream.MakeFromLast(span); }
		TextSpan MakeFromLast(const Token& token) const { return tokenStream.MakeFromLast(token); }

		bool HasCriticalErrors() const noexcept { return tokenStream.HasCriticalErrors(); }

		// Restore points
		void MakeRestorePoint() { tokenStream.MakeRestorePoint(); }
		void RestoreFromPoint() { tokenStream.RestoreFromPoint(); }

		void PushState() { tokenStream.PushState(); }
		void PopState(bool restore = true) { tokenStream.PopState(restore); }

		// Token advancing
		void Advance() { tokenStream.Advance(); }
		bool TryAdvance() { return tokenStream.TryAdvance(); }

		// Token retrieval
		bool TryGetToken(TokenType type, Token& current) const { return tokenStream.TryGetToken(type, current); }

		bool TryGetTypeValue(TokenType type, int value) { return tokenStream.TryGetTypeValue(type, value); }

		bool TryGetTypeAnyValue(TokenType type, int& value) { return tokenStream.TryGetTypeAnyValue(type, value); }

		bool TryGetDelimiter(char delimiter) { return tokenStream.TryGetDelimiter(delimiter); }

		bool TryGetKeyword(Keyword keyword) { return tokenStream.TryGetKeyword(keyword); }

		bool TryGetKeywords(const std::unordered_set<Keyword>& keywords) { return tokenStream.TryGetKeywords(keywords); }

		bool TryGetOperator(Operator op) { return tokenStream.TryGetOperator(op); }

		bool TryGetUnaryOperator(Operator& op) { return tokenStream.TryGetUnaryOperator(op); }

		bool TryGetAnyOperator(Operator& op) { return tokenStream.TryGetAnyOperator(op); }

		bool TryGetIdentifier(TextSpan& span) { return tokenStream.TryGetIdentifier(span); }

		bool TryGetLiteral(TextSpan& span) { return tokenStream.TryGetLiteral(span); }

		bool TryGetNumber(Number& number) { return tokenStream.TryGetNumber(number); }

		bool Expect(TokenType type, Token& current, bool advance = true)
		{
			return tokenStream.Expect(type, current, advance);
		}

		template <typename... Args>
		bool ExpectOperator(Operator op, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectOperator(op, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectAnyOperator(Operator& op, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectAnyOperator(op, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectLiteral(TextSpan& literal, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectLiteral(literal, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectIdentifier(TextSpan& identifier, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectIdentifier(identifier, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectKeyword(const Keyword& keyword, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectKeyword(keyword, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectKeywords(const std::unordered_set<Keyword>& keywords, Keyword& keyword, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectKeywords(keywords, keyword, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectDelimiter(char delimiter, Token& token, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectDelimiter(delimiter, token, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectDelimiter(char delimiter, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectDelimiter(delimiter, code, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectNumeric(Number& numberOut, Token& token, const std::string& message, Args&&... args)
		{
			return tokenStream.ExpectNumeric(numberOut, token, message, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectNumeric(Number& numberOut, const std::string& message, Args&&... args)
		{
			return tokenStream.ExpectNumeric(numberOut, message, std::forward<Args>(args)...);
		}

		template <typename... Args>
		bool ExpectNoDelimiters(const std::unordered_set<char>& delimiters, DiagnosticCode code, Args&&... args)
		{
			return tokenStream.ExpectNoDelimiters(delimiters, code, std::forward<Args>(args)...);
		}
	};
}
#endif