#ifndef PARSER_HELPER_HPP
#define PARSER_HELPER_HPP
#include "sub_parser.hpp"

namespace ParserHelper
{
	using namespace HXSL;

	bool TryParseMemberAccessPath(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut);

	bool ParseFunctionCallInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::vector<std::unique_ptr<CallParameter>>& parameters);

	bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression);

	bool TryParseFunctionCall(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression);

	bool TryParseSymbol(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut);

	bool TryParseLiteralExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<LiteralExpression>& expressionOut);

	bool TryParseInitializationExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<InitializationExpression>& expressionOut);
}

#endif