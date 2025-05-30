#include "interfaces.hpp"
#include "expressions.hpp"
#include "statements.hpp"

namespace HXSL
{
	void IHasExpressions::RegisterExpression(ASTNode* parent, Expression* expr)
	{
		if (expr == nullptr) return;
		expressions.push_back(expr);
		expr->SetParent(parent);
	}

	void IHasExpressions::UnregisterExpression(Expression* expr)
	{
		if (expr == nullptr) return;
		expressions.erase(remove(expressions.begin(), expressions.end(), expr), expressions.end());
		expr->SetParent(nullptr);
	}
}