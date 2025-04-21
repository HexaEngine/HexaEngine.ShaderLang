#ifndef SUB_PARSER_H
#define SUB_PARSER_H

#include "parser.h"
#include "compilation.hpp"
#include "token_stream.h"
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

#define IF_POP_RET_FALSE(stream, expr) \
	if (expr) { \
		stream.PopState(); \
		return false; \
	}

	class HXSLSubParser
	{
	protected:
		HXSLSubParser()
		{
		}
	public:
		virtual bool TryParse(HXSLParser& parser, TokenStream& stream, Compilation* compilation)
		{
			return false;
		}
	};

	class HXSLStatementParser
	{
	protected:
		HXSLStatementParser()
		{
		}
	public:
		virtual bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
		{
			return false;
		}
	};

	class HXSLExpressionParser
	{
	protected:
		HXSLExpressionParser()
		{
		}
	public:
		virtual bool TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
		{
			return false;
		}
	};
}

#endif