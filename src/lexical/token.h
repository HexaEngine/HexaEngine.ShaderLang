#ifndef TOKEN_H
#define TOKEN_H

#include "log.h"
#include "text_span.h"
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

	static std::string ToString(TokenType type) {
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
			HXSLNumber Numeric;
		};

		Token() : Span(TextSpan()), Type(TokenType_Unknown), Numeric(HXSLNumber()) {}

		Token(TextSpan span, TokenType type) : Span(span), Type(type), Numeric(HXSLNumber()) {}

		Token(TextSpan span, TokenType type, int value) : Span(span), Type(type), Value(value) {}

		Token(TextSpan span, TokenType type, HXSLNumber number) : Span(span), Type(type), Numeric(number) {}

		bool isKeywordOf(std::initializer_list<int> keywords) const
		{
			if (Type != TokenType_Keyword) return false;
			for (int keyword : keywords)
			{
				if (Value == keyword)
					return true;
			}
			return false;
		}

		bool isDelimiterOf(char delimiter) const
		{
			if (Type != TokenType_Delimiter) return false;
			return Span[0] == delimiter;
		}

		bool isDelimiterOf(std::initializer_list<char> delimiters) const
		{
			if (Type != TokenType_Delimiter) return false;
			char first = Span[0];
			for (char d : delimiters)
			{
				if (first == d)
					return true;
			}
			return false;
		}

		bool isOperator() const noexcept { return Type == TokenType_Operator; }

		bool isOperatorOf(HXSLOperator op) const noexcept { return Type == TokenType_Operator && op == static_cast<HXSLOperator>(Value); }

		bool isIdentifier() const noexcept { return Type == TokenType_Identifier; }

		bool isLiteral() const noexcept { return Type == TokenType_Literal; }

		bool isNumeric() const noexcept { return Type == TokenType_Numeric; }

		bool isOperator(HXSLOperator& op) const noexcept
		{
			op = static_cast<HXSLOperator>(Value);
			return Type == TokenType_Operator;
		}

		bool isUnaryOperator(HXSLOperator& op) const noexcept
		{
			op = static_cast<HXSLOperator>(Value);
			return Type == TokenType_Operator && Operators::isUnaryOperator(op);
		}

		bool isCompoundAssignment(HXSLOperator& op) const noexcept
		{
			op = static_cast<HXSLOperator>(Value);
			return Type == TokenType_Operator && Operators::isCompoundAssignment(op);
		}

		bool isCompoundAssignment() const noexcept
		{
			return Type == TokenType_Operator && Operators::isCompoundAssignment(static_cast<HXSLOperator>(Value));
		}
	};
}

#endif