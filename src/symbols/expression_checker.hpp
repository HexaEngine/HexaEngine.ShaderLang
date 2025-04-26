#ifndef EXPRESSION_CHECKER_HPP
#define EXPRESSION_CHECKER_HPP

#include "ast_analyzers.hpp"

namespace HXSL
{
	struct Analyzer;
	class TypeChecker;
	class SymbolResolver;

	class ExpressionChecker
	{
	public:
		virtual void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack) = 0;
	};

	class ExpressionCheckerRegistry
	{
	private:
		static std::unique_ptr<ExpressionChecker> handlers[NodeType_ExpressionCount];
		static std::once_flag initFlag;
	public:
		static void EnsureCreated();

		static ExpressionChecker* GetHandler(NodeType type)
		{
			if (type >= NodeType_ExpressionFirst && type <= NodeType_ExpressionLast)
			{
				return handlers[type - NodeType_ExpressionFirst].get();
			}
			return nullptr;
		}

		template <typename CheckerType, NodeType Type>
		static typename std::enable_if<std::is_base_of<ExpressionChecker, CheckerType>::value>::type
			Register()
		{
			handlers[Type - NodeType_ExpressionFirst] = std::make_unique<CheckerType>();
		}
	};

	class BinaryExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class UnaryExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class CastExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class ComplexMemberAccessExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class MemberAccessExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class MemberReferenceExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class FunctionCallExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class TernaryExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class LiteralExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class PrefixPostfixExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};

	class IndexerExpressionChecker : public ExpressionChecker
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack);
	};
}

#endif