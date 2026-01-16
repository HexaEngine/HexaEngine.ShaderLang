#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "ast_base.hpp"

#include <memory>
#include <vector>

namespace HXSL
{
#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { UnregisterExpression(field); field = value; RegisterExpression(this, field); } \
	type Detach##name() noexcept { UnregisterExpression(field); return std::move(field); }

	class IHasExpressions
	{
	private:
		std::vector<Expression*> expressions;

	protected:
		void RegisterExpression(ASTNode* parent, Expression* expr);

		template<class T>
		void RegisterExpressions(ASTNode* parent, const std::vector<ast_ptr<T>>& expressions)
		{
			for (auto& expr : expressions)
			{
				RegisterExpression(parent, expr.get());
			}
		}

		void UnregisterExpression(Expression* expr);

		template<class T>
		void UnregisterExpression(const std::vector<ast_ptr<T>>& expressions)
		{
			for (auto& expr : expressions)
			{
				UnregisterExpression(expr.get());
			}
		}

	public:
		const std::vector<Expression*>& GetExpressions() const noexcept
		{
			return expressions;
		}

		virtual ~IHasExpressions() = default;
	};
}

#endif