#ifndef HYBRID_EXPRESSION_PARSER_HPP
#define HYBRID_EXPRESSION_PARSER_HPP

#include "parser.hpp"

namespace HXSL
{
	enum ExpressionParserFlags
	{
		ExpressionParserFlags_None = 0,
		ExpressionParserFlags_SwitchCase = 1,
	};

	DEFINE_FLAGS_OPERATORS(ExpressionParserFlags, int);

	class HybridExpressionParser
	{
	public:
		static bool ParseExpression(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression, ExpressionParserFlags flags = ExpressionParserFlags_None);
	};
}

#endif