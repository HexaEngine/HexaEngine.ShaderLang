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
		void RegisterExpression(ASTNode* parent, Expression* expr);

		template<class T>
		void RegisterExpressions(ASTNode* parent, const std::vector<std::unique_ptr<T>>& expressions)
		{
			for (auto& expr : expressions)
			{
				RegisterExpression(parent, expr.get());
			}
		}

		void UnregisterExpression(Expression* expr);

		template<class T>
		void UnregisterExpression(const std::vector<std::unique_ptr<T>>& expressions)
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