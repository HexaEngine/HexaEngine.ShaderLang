#include "sub_parser_registry.hpp"

#include "declaration_parser.hpp"
#include "statement_parser.hpp"
#include "expression_parser.hpp"

namespace HXSL
{
	std::vector<std::unique_ptr<SubParser>> SubParserRegistry::parsers;
	std::once_flag SubParserRegistry::initFlag;
	std::vector<std::unique_ptr<StatementParser>> StatementParserRegistry::parsers;
	std::once_flag StatementParserRegistry::initFlag;
	std::vector<std::unique_ptr<ExpressionParser>> ExpressionParserRegistry::parsers;
	std::once_flag ExpressionParserRegistry::initFlag;

	void SubParserRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<StructParser>();
				Register<OperatorParser>();
				Register<DeclarationParser>();
				Register<UsingParser>();
				Register<NamespaceParser>();
			});
	}

	void StatementParserRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<BlockStatementParser>();
				Register<MiscKeywordStatementParser>();
				Register<SwitchStatementParser>();
				Register<ForStatementParser>();
				Register<WhileStatementParser>();
				Register<DoWhileStatementParser>();
				Register<IfStatementParser>();
				Register<ReturnStatementParser>();
				Register<MemberAccessStatementParser>();
			});
	}

	void ExpressionParserRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<LiteralExpressionParser>();
				Register<CtorCallExpressionParser>();
				Register<MemberAccessExpressionParser>();
				Register<SymbolExpressionParser>();
			});
	}
}