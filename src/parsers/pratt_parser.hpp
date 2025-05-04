#ifndef PRATT_PARSER_HPP
#define PRATT_PARSER_HPP

#include "parser.hpp"

namespace HXSL
{
	class PrattParser
	{
	public:
		static bool ParseExpressionInnerIterGen2(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression);

		static bool ParseExpression(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression);
	};
}

#endif