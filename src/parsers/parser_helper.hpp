#ifndef PARSER_HELPER_HPP
#define PARSER_HELPER_HPP
#include "sub_parser.hpp"

namespace ParserHelper
{
	using namespace HXSL;

	bool TryParseMemberAccessPath(HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<HXSLExpression>& expressionOut);

	bool ParseFunctionCallInner(HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::vector<std::unique_ptr<HXSLCallParameter>>& parameters);

	bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<HXSLFunctionCallExpression>& expression);

	bool TryParseFunctionCall(HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<HXSLFunctionCallExpression>& expression);

	bool TryParseSymbol(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut);

	bool TryParseLiteralExpression(HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<HXSLLiteralExpression>& expressionOut);

	bool TryParseInitializationExpression(HXSLParser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<HXSLInitializationExpression>& expressionOut);
}

#endif