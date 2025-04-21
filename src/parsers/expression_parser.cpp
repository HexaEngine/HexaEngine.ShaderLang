#include "expression_parser.hpp"

#include "sub_parser_registry.hpp"
#include "statement_parser.hpp"
#include "parser.h"
#include "nodes.hpp"
#include "parser_helper.hpp"
#include "pratt_parser.hpp"
namespace HXSL
{
	bool HXSLSymbolExpressionParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		return ParserHelper::TryParseSymbol(parser, stream, expressionOut);
	}

	bool HXSLLiteralExpressionParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		std::unique_ptr<HXSLLiteralExpression> expr;
		IF_ERR_RET_FALSE(ParserHelper::TryParseLiteralExpression(parser, stream, parser.parentNode(), expr));
		expressionOut = std::move(expr);
		return true;
	}

	bool HXSLFuncCallExpressionParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		auto current = stream.Current();

		std::unique_ptr<HXSLFunctionCallExpression> expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseFunctionCall(parser, stream, parser.parentNode(), expression));
		expressionOut = std::move(expression);

		return true;
	}

	bool HXSLMemberAccessExpressionParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		auto start = stream.Current();
		std::unique_ptr<HXSLExpression> expression;
		IF_ERR_RET_FALSE(ParserHelper::TryParseMemberAccessPath(parser, stream, parser.parentNode(), expression));
		expressionOut = std::move(expression);
		return true;
	}

	bool HXSLAssignmentExpressionParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		auto start = stream.Current();
		std::unique_ptr<HXSLExpression> target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, nullptr, target))
		{
			return false;
		}

		if (stream.TryGetOperator(HXSLOperator_Assign))
		{
			auto assignmentStatement = std::make_unique<HXSLAssignmentExpression>(TextSpan(), parser.parentNode(), std::move(target), nullptr);
			std::unique_ptr<HXSLExpression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, assignmentStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			assignmentStatement->SetExpression(std::move(expression));
			assignmentStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			expressionOut = std::move(assignmentStatement);
			return true;
		}

		auto token = stream.Current();
		HXSLOperator op;
		if (token.isCompoundAssignment(op))
		{
			stream.Advance();
			auto assignmentStatement = std::make_unique<HXSLCompoundAssignmentExpression>(TextSpan(), parser.parentNode(), op, std::move(target), nullptr);
			std::unique_ptr<HXSLExpression> expression;
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