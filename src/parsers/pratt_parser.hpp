#ifndef PRATT_PARSER_H
#define PRATT_PARSER_H

#include "sub_parser_registry.hpp"

namespace HXSL
{
	using ExpressionPtr = std::unique_ptr<Expression>;

	class PrattParser
	{
	public:
		static bool TryParseUnaryPrefixOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets);

		static bool TryParseUnaryPostfixOperator(const Token& start, Operator op, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool TryParseCastOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool ParseExpressionInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, int precedence = 0);

		static bool ParseExpressionInnerIterGen2(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool ParseOperand(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets);

		static bool ParseExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);
	};
}

#endif