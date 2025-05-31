#ifndef IL_BUILDER_HELPER_HPP
#define IL_BUILDER_HELPER_HPP

#include "pch/ast.hpp"

namespace HXSL
{
	static bool IsImmediate(Expression* expr, Number& num)
	{
		if (auto literal = dyn_cast<LiteralExpression>(expr))
		{
			auto& token = literal->GetLiteral();
			if (token.isNumeric())
			{
				num = token.Numeric;
			}
			else if (token.isBool())
			{
				num = Number(token.Value == Keyword_True);
			}
			else
			{
				HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
			}

			return true;
		}
		return false;
	}
}

#endif