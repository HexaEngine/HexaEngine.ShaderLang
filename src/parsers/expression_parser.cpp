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

	bool AssignmentExpressionParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		auto start = stream.Current();
		std::unique_ptr<Expression> target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, nullptr, target))
		{
			return false;
		}

		if (stream.TryGetOperator(Operator_Assign))
		{
			auto assignmentStatement = std::make_unique<AssignmentExpression>(TextSpan(), parser.parentNode(), std::move(target), nullptr);
			std::unique_ptr<Expression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, assignmentStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			assignmentStatement->SetExpression(std::move(expression));
			assignmentStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			expressionOut = std::move(assignmentStatement);
			return true;
		}

		auto token = stream.Current();
		Operator op;
		if (token.isCompoundAssignment(op))
		{
			stream.Advance();
			auto assignmentStatement = std::make_unique<CompoundAssignmentExpression>(TextSpan(), parser.parentNode(), op, std::move(target), nullptr);
			std::unique_ptr<Expression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, assignmentStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			assignmentStatement->SetExpression(std::move(expression));
			assignmentStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			expressionOut = std::move(assignmentStatement);
			return true;
		}

		return false;
	}
}