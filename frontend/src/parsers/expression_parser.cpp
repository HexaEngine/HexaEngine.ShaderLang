#include "expression_parser.hpp"

#include "sub_parser_registry.hpp"
#include "statement_parser.hpp"
#include "parsers/parser.hpp"
#include "pch/ast.hpp"
#include "parser_helper.hpp"
#include "hybrid_expr_parser.hpp"
namespace HXSL
{
	bool SymbolExpressionParser::TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut)
	{
		return ParserHelper::TryParseSymbol(parser, stream, expressionOut);
	}

	bool LiteralExpressionParser::TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut)
	{
		LiteralExpression* expr;
		IF_ERR_RET_FALSE(ParserHelper::TryParseLiteralExpression(parser, stream, expr));
		expressionOut = expr;
		return true;
	}

	bool FuncCallExpressionParser::TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut)
	{
		auto current = stream.Current();

		FunctionCallExpression* expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseFunctionCall(parser, stream, expression));
		expressionOut = expression;

		return true;
	}

	bool MemberAccessExpressionParser::TryParse(Parser& parser, TokenStream& stream, Expression*& expressionOut)
	{
		auto start = stream.Current();
		Expression* expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseMemberAccessPath(parser, stream, expression));
		expressionOut = expression;
		return true;
	}
}