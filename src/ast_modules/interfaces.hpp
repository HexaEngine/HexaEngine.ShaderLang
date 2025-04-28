#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "ast_base.hpp"

#include <memory>
#include <vector>

namespace HXSL
{
	class IHasSymbolRef
	{
	public:
		virtual const std::unique_ptr<SymbolRef>& GetSymbolRef() = 0;
		virtual ~IHasSymbolRef() = default;
	};

	class IHasExpressions
	{
	private:
		std::vector<Expression*> expressions;

	protected:
		void RegisterExpression(Expression* expr)
		{
			if (expr == nullptr) return;
			expressions.push_back(expr);
		}

		void UnregisterExpression(Expression* expr)
		{
			if (expr == nullptr) return;
			expressions.erase(remove(expressions.begin(), expressions.end(), expr), expressions.end());
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
		virtual const std::vector<std::unique_ptr<OperatorOverload>>& GetOperators() const = 0;
		virtual ~IHasOperatorOverloads() = default;
	};

	class ICloneable
	{
	public:
		virtual std::unique_ptr<ASTNode> Clone(ASTNode* parent) const noexcept = 0;
		virtual ~ICloneable() = default;
	};
}

#endif