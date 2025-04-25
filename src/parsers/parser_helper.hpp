#ifndef PARSER_HELPER_HPP
#define PARSER_HELPER_HPP
#include "sub_parser.hpp"

namespace HXSL
{
	class ParserHelper
	{
	public:
		static bool TryParseMemberAccessPath(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut);

		static bool ParseFunctionCallInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::vector<std::unique_ptr<FunctionCallParameter>>& parameters);

		static bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression);

		static bool TryParseFunctionCall(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression);

		static bool TryParseSymbol(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut);

		static bool TryParseLiteralExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<LiteralExpression>& expressionOut);

		static bool TryParseInitializationExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<InitializationExpression>& expressionOut);

		static bool MakeConcreteSymbolRef(Expression* expression, SymbolRefType type, std::unique_ptr<SymbolRef>& symbolOut);
	};
}

#endif