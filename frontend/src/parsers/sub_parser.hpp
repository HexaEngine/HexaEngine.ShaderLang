#ifndef SUB_PARSER_H
#define SUB_PARSER_H

#include "parsers/parser.hpp"
#include "lexical/token_stream.hpp"

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
		virtual bool TryParse(Parser& parser, TokenStream& stream) = 0;
	};

	class StatementParser
	{
	protected:
		StatementParser()
		{
		}
	public:
		virtual bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut) = 0;
	};

	class ExpressionParser
	{
	protected:
		ExpressionParser()
		{
		}
	public:
		virtual bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut) = 0;
	};
}

#endif