#ifndef SUB_PARSER_H
#define SUB_PARSER_H

#include "parsers/parser.hpp"
#include "lexical/token_stream.h"
#include <memory>
#include <iostream>

namespace HXSL
{
#define IF_RET_FALSE(expr) \
	if (expr) { \
	 return false; \
	}

#define IF_ERR_RET_FALSE(expr) \
	if (!expr) { \
	 return false; \
	}

#define IF_ERR_RET(expr) \
	if (!expr) { \
	 return; \
	}

#define IF_POP_RET_FALSE(stream, expr) \
	if (expr) { \
		stream.PopState(); \
		return false; \
	}

	class SubParser
	{
	protected:
		SubParser()
		{
		}
	public:
		virtual bool TryParse(Parser& parser, TokenStream& stream, Compilation* compilation)
		{
			return false;
		}
	};

	class StatementParser
	{
	protected:
		StatementParser()
		{
		}
	public:
		virtual bool TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
		{
			return false;
		}
	};

	class ExpressionParser
	{
	protected:
		ExpressionParser()
		{
		}
	public:
		virtual bool TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
		{
			return false;
		}
	};
}

#endif