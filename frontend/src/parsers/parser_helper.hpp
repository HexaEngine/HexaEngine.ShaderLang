#ifndef PARSER_HELPER_HPP
#define PARSER_HELPER_HPP

#include "sub_parser.hpp"

namespace HXSL
{
	class ParserHelper
	{
	public:
		static bool TryParseMemberAccessPath(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut, Expression** headOut = nullptr);

		static bool ParseFunctionCallInner(Parser& parser, TokenStream& stream, std::vector<ast_ptr<FunctionCallParameter>>& parameters);

		static bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, Parser& parser, TokenStream& stream, ast_ptr<FunctionCallExpression>& expression);

		static bool TryParseFunctionCall(Parser& parser, TokenStream& stream, ast_ptr<FunctionCallExpression>& expression);

		static bool TryParseSymbol(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut);

		static bool TryParseLiteralExpression(Parser& parser, TokenStream& stream, ast_ptr<LiteralExpression>& expressionOut);

		static bool TryParseInitializationExpression(Parser& parser, TokenStream& stream, ast_ptr<InitializationExpression>& expressionOut);

		static bool MakeConcreteSymbolRef(Expression* expression, SymbolRefType type, ast_ptr<SymbolRef>& symbolOut);
	};
}

#endif