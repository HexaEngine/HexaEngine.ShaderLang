#ifndef DECLARATION_PARSER_HPP
#define DECLARATION_PARSER_HPP

#include "sub_parser.hpp"

namespace HXSL
{
	class NamespaceParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut) override;
	};

	class UsingParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut) override;
	};

	class DeclarationParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut) override;
	};

	class OperatorParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut) override;
	};

	class StructParser : public SubParser
	{
		bool TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut) override;
	};
}

#endif