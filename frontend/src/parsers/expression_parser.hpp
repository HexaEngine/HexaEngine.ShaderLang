#ifndef FUNC_CALL_EXPR_PARSER
#define FUNC_CALL_EXPR_PARSER

#include "sub_parser_registry.hpp"
#include "sub_parser.hpp"
namespace HXSL
{
	class SymbolExpressionParser : public ExpressionParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut) override;
	};

	class LiteralExpressionParser : public ExpressionParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut) override;
	};

	class FuncCallExpressionParser : public ExpressionParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut) override;
	};

	class CtorCallExpressionParser : public ExpressionParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut) override;
	};

	class MemberAccessExpressionParser : public ExpressionParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut) override;
	};
}

#endif