#ifndef FUNC_CALL_EXPR_PARSER
#define FUNC_CALL_EXPR_PARSER

#include "sub_parser_registry.hpp"
#include "sub_parser.hpp"
namespace HXSL
{
	class HXSLSymbolExpressionParser : public HXSLExpressionParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut) override;
	};

	class HXSLLiteralExpressionParser : public HXSLExpressionParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut) override;
	};

	class HXSLFuncCallExpressionParser : public HXSLExpressionParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut) override;
	};

	class HXSLMemberAccessExpressionParser : public HXSLExpressionParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut) override;
	};

	class HXSLAssignmentExpressionParser : public HXSLExpressionParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut) override;
	};
}

#endif