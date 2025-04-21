#ifndef STATEMENT_PARSER
#define STATEMENT_PARSER

#include "sub_parser.hpp"
namespace HXSL
{
	class HXSLMiscKeywordStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLBlockStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLForStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLSwitchStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLIfStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLElseStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLWhileStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLReturnStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLDeclarationStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLAssignmentStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};

	class HXSLFunctionCallStatementParser : public HXSLStatementParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut) override;
	};
}

#endif