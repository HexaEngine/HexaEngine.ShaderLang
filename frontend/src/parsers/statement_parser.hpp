#ifndef STATEMENT_PARSER
#define STATEMENT_PARSER

#include "sub_parser.hpp"
namespace HXSL
{
	class MiscKeywordStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class BlockStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class ForStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class SwitchStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class IfStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class WhileStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class DoWhileStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class ReturnStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};

	class MemberAccessStatementParser : public StatementParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) override;
	};
}

#endif