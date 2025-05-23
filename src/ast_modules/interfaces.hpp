#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "ast_base.hpp"

#include <memory>
#include <vector>

namespace HXSL
{
#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterExpression(field.get()); field = std::move(value); RegisterExpression(this, field.get()); } \
	type Detach##name() noexcept { UnregisterExpression(field.get()); return std::move(field); }

	class IHasSymbolRef
	{
	public:
		virtual const ast_ptr<SymbolRef>& GetSymbolRef() = 0;
		virtual ~IHasSymbolRef() = default;
	};

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

	class IHasOperatorOverloads
	{
	public:
		virtual const std::vector<ast_ptr<OperatorOverload>>& GetOperators() const = 0;
		virtual ~IHasOperatorOverloads() = default;
	};

	class ICloneable
	{
	public:
		virtual ast_ptr<ASTNode> Clone(ASTNode* parent) const noexcept = 0;
		virtual ~ICloneable() = default;
	};

	class IHasBody
	{
	public:
		virtual const ast_ptr<BlockStatement>& GetBody() const noexcept = 0;
		virtual ~IHasBody() = default;
	};

	class IHasCanonicalParent
	{
	protected:
		ASTNode* canonicalParent;
	public:
		ASTNode* GetCanonicalParent() const noexcept { return canonicalParent; }
		void SetCanonicalParent(ASTNode* newParent) { canonicalParent = newParent; }
	};
}

#endif