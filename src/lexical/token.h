#ifndef TOKEN_H
#define TOKEN_H

#include "lexical/text_span.hpp"
#include "numbers.h"
#include "lang/language.h"

namespace HXSL
{
	enum TokenType
	{
		TokenType_Unknown,
		TokenType_Keyword,
		TokenType_Identifier,
		TokenType_Literal,
		TokenType_Numeric,
		TokenType_Codeblock,
		TokenType_Delimiter,
		TokenType_Operator,
		TokenType_Comment,
		TokenType_NewLine,
		TokenType_Whitespace
	};

	static std::string ToString(TokenType type)
	{
		switch (type) {
		case TokenType_Unknown: return "Unknown";
		case TokenType_Keyword: return "Keyword";
		case TokenType_Identifier: return "Identifier";
		case TokenType_Literal: return "Literal";
		case TokenType_Numeric: return "Numeric";
		case TokenType_Codeblock: return "Codeblock";
		case TokenType_Delimiter: return "Delimiter";
		case TokenType_Operator: return "Operator";
		case TokenType_Comment: return "Comment";
		case TokenType_NewLine: return "New Line";
		case TokenType_Whitespace: return "Whitespace";
		default: return "Invalid TokenType";
		}
	}

	struct Token
	{
		TextSpan Span;
		TokenType Type;
		union
		{
			int Value;
			Number Numeric;
		};

		Token() : Span(TextSpan()), Type(TokenType_Unknown), Numeric(Number()) {}

		Token(TextSpan span, TokenType type) : Span(span), Type(type), Numeric(Number()) {}

		Token(TextSpan span, TokenType type, int value) : Span(span), Type(type), Value(value) {}

		Token(TextSpan span, TokenType type, Number number) : Span(span), Type(type), Numeric(number) {}

		Keyword asKeyword() const noexcept
		{
			return  static_cast<Keyword>(Value);
		}

		Operator asOperator() const noexcept
		{
			return  static_cast<Operator>(Value);
		}

		bool isNewLine() const noexcept { return Type == TokenType_NewLine; }

		bool isKeyword() const noexcept { return Type == TokenType_Keyword; }

		bool isKeywordOf(const Keyword& keyword) const
		{
			if (!isKeyword()) return false;
			return static_cast<Keyword>(Value) == keyword;
		}

		bool isKeywordOf(const std::unordered_set<Keyword>& keywords) const
		{
			if (!isKeyword()) return false;
			auto k = static_cast<Keyword>(Value);
			return keywords.find(k) != keywords.end();
		}

		bool isDelimiter() const noexcept { return Type == TokenType_Delimiter; }

		bool isDelimiterOf(const char& delimiter) const
		{
			if (!isDelimiter()) return false;
			return Value == delimiter;
		}

		bool isDelimiterOf(const std::unordered_set<char>& delimiters) const
		{
			if (!isDelimiter()) return false;
			char k = Value;
			return delimiters.find(k) != delimiters.end();
		}

		bool isOperator() const noexcept { return Type == TokenType_Operator; }

		bool isOperator(Operator& opOut) const noexcept
		{
			opOut = static_cast<Operator>(Value);
			return Type == TokenType_Operator;
		}

		bool isOperatorOf(const Operator& op) const noexcept
		{
			if (!isOperator()) return false;
			return static_cast<Operator>(Value) == op;
		}

		bool isOperatorOf(const std::unordered_set<Operator>& ops) const noexcept
		{
			if (!isOperator()) return false;
			auto k = static_cast<Operator>(Value);
			return ops.find(k) != ops.end();
		}

		bool isUnaryOperator(Operator& op) const noexcept
		{
			op = static_cast<Operator>(Value);
			return Type == TokenType_Operator && Operators::isUnaryOperator(op);
		}

		bool isAssignment(Operator& op) const noexcept
		{
			op = static_cast<Operator>(Value);
			return Type == TokenType_Operator && Operators::isAssignment(op);
		}

		bool isAssignment() const noexcept
		{
			return Type == TokenType_Operator && Operators::isAssignment(static_cast<Operator>(Value));
		}

		bool isCompoundAssignment(Operator& op) const noexcept
		{
			op = static_cast<Operator>(Value);
			return Type == TokenType_Operator && Operators::isCompoundAssignment(op);
		}

		bool isCompoundAssignment() const noexcept
		{
			return Type == TokenType_Operator && Operators::isCompoundAssignment(static_cast<Operator>(Value));
		}

		bool isIdentifier() const noexcept { return Type == TokenType_Identifier; }

		bool isLiteral() const noexcept { return Type == TokenType_Literal; }

		bool isNumeric() const noexcept { return Type == TokenType_Numeric; }
	};
}

#endif