#ifndef DECLARATION_PARSER_HPP
#define DECLARATION_PARSER_HPP

#include "sub_parser.hpp"

namespace HXSL
{
	class DeclarationParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream) override;
	};

	class OperatorParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream) override;
	};

	class StructParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream) override;
	};
}

#endif