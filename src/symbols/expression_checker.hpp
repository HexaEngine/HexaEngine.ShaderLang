#ifndef EXPRESSION_CHECKER_HPP
#define EXPRESSION_CHECKER_HPP

#include "ast_analyzers.hpp"

namespace HXSL
{
	struct Analyzer;
	class TypeChecker;
	class SymbolResolver;

	class ExpressionCheckerBase
	{
	public:
		virtual void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack) = 0;
	};

	template<class T>
	class ExpressionChecker : public ExpressionCheckerBase
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
		{
			T* expression = static_cast<T*>(node);
			HandleExpression(analyzer, checker, resolver, expression, stack);
		}

		virtual void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, T* expression, std::stack<Expression*>& stack) = 0;
	};

	class ExpressionCheckerRegistry
	{
	private:
		static std::unique_ptr<ExpressionCheckerBase> handlers[NodeType_ExpressionCount];
		static std::once_flag initFlag;
	public:
		static void EnsureCreated();

		static ExpressionCheckerBase* GetHandler(NodeType type)
		{
			if (type >= NodeType_FirstExpression && type <= NodeType_LastExpression)
			{
				return handlers[type - NodeType_FirstExpression].get();
			}
			return nullptr;
		}

		template <typename CheckerType, NodeType Type>
		static typename std::enable_if<std::is_base_of<ExpressionCheckerBase, CheckerType>::value>::type
			Register()
		{
			handlers[Type - NodeType_FirstExpression] = std::make_unique<CheckerType>();
		}
	};

	class DummyExpressionChecker : public ExpressionCheckerBase
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack) override
		{
		}
	};

	class BinaryExpressionChecker : public ExpressionChecker<BinaryExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, BinaryExpression* expression, std::stack<Expression*>& stack);
	};

	class UnaryExpressionChecker : public ExpressionChecker<UnaryExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack);
	};

	class CastExpressionChecker : public ExpressionChecker<CastExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CastExpression* expression, std::stack<Expression*>& stack);
	};

	class MemberAccessExpressionChecker : public ExpressionChecker<MemberAccessExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberAccessExpression* expression, std::stack<Expression*>& stack);
	};

	class MemberReferenceExpressionChecker : public ExpressionChecker<MemberReferenceExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberReferenceExpression* expression, std::stack<Expression*>& stack);
	};

	class FunctionCallExpressionChecker : public ExpressionChecker<FunctionCallExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, FunctionCallExpression* expression, std::stack<Expression*>& stack);
	};

	class TernaryExpressionChecker : public ExpressionChecker<TernaryExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, TernaryExpression* expression, std::stack<Expression*>& stack);
	};

	class LiteralExpressionChecker : public ExpressionChecker<LiteralExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, LiteralExpression* expression, std::stack<Expression*>& stack);
	};

	class PrefixPostfixExpressionChecker : public ExpressionChecker<UnaryExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack);
	};

	class IndexerExpressionChecker : public ExpressionChecker<IndexerAccessExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, IndexerAccessExpression* expression, std::stack<Expression*>& stack);
	};

	class AssignmentChecker : public ExpressionChecker<AssignmentExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, AssignmentExpression* expression, std::stack<Expression*>& stack);
	};

	class CompoundAssignmentChecker : public ExpressionChecker<CompoundAssignmentExpression>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CompoundAssignmentExpression* expression, std::stack<Expression*>& stack);
	};
}

#endif