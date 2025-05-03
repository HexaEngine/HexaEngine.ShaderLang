#ifndef PRATT_PARSER_H
#define PRATT_PARSER_H

#include "sub_parser_registry.hpp"

namespace HXSL
{
	using ExpressionPtr = std::unique_ptr<Expression>;

	class PrattParser
	{
	public:
		static bool ParseExpressionInnerIterGen2(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression);

		static bool ParseExpression(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression);
	};
}

#endif