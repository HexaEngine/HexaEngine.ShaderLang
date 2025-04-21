#ifndef DECLARATION_PARSER_HPP
#define DECLARATION_PARSER_HPP

#include "sub_parser.hpp"

namespace HXSL
{
	class HXSLDeclarationParser : public HXSLSubParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, Compilation* compilation) override;
	};

	class HXSLStructParser : public HXSLSubParser
	{
		bool TryParse(HXSLParser& parser, TokenStream& stream, Compilation* compilation) override;
	};
}

#endif