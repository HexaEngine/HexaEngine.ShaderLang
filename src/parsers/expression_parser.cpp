#include "expression_parser.hpp"

#include "sub_parser_registry.hpp"
#include "statement_parser.hpp"
#include "parsers/parser.h"
#include "ast.hpp"
#include "parser_helper.hpp"
#include "pratt_parser.hpp"
namespace HXSL
{
	bool SymbolExpressionParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		return ParserHelper::TryParseSymbol(parser, stream, expressionOut);
	}

	bool LiteralExpressionParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		std::unique_ptr<LiteralExpression> expr;
		IF_ERR_RET_FALSE(ParserHelper::TryParseLiteralExpression(parser, stream, parser.parentNode(), expr));
		expressionOut = std::move(expr);
		return true;
	}

	bool FuncCallExpressionParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		auto current = stream.Current();

		std::unique_ptr<FunctionCallExpression> expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseFunctionCall(parser, stream, parser.parentNode(), expression));
		expressionOut = std::move(expression);

		return true;
	}

	bool MemberAccessExpressionParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		auto start = stream.Current();
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseMemberAccessPath(parser, stream, parser.parentNode(), expression));
		expressionOut = std::move(expression);
		return true;
	}
}