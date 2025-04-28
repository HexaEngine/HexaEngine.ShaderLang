#ifndef STATEMENT_CHECKER_HPP
#define STATEMENT_CHECKER_HPP

#include "ast_analyzers.hpp"

namespace HXSL
{
	struct Analyzer;
	class TypeChecker;
	class SymbolResolver;

	class StatementCheckerBase
	{
	public:
		virtual void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Statement* node) = 0;
	};

	template<class T>
	class StatementChecker : public StatementCheckerBase
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Statement* node)
		{
			T* statement = static_cast<T*>(node);
			HandleExpression(analyzer, checker, resolver, statement);
		}

		virtual void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, T* statement) = 0;
	};

	class StatementCheckerRegistry
	{
	private:
		static std::unique_ptr<StatementCheckerBase> handlers[NodeType_StatementCount];
		static std::once_flag initFlag;
	public:
		static void EnsureCreated();

		static StatementCheckerBase* GetHandler(NodeType type)
		{
			if (type >= NodeType_FirstStatement && type <= NodeType_LastStatement)
			{
				return handlers[type - NodeType_FirstStatement].get();
			}
			return nullptr;
		}

		template <typename CheckerType, NodeType Type>
		static typename std::enable_if<std::is_base_of<StatementCheckerBase, CheckerType>::value>::type
			Register()
		{
			handlers[Type - NodeType_FirstStatement] = std::make_unique<CheckerType>();
		}
	};

	class DummyStatementChecker : public StatementCheckerBase
	{
		 void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Statement* node) override
		 {
		 }
	};

	class ReturnStatementChecker : public StatementChecker<ReturnStatement>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ReturnStatement* statement) override;
	};

	class DeclarationStatementChecker : public StatementChecker<DeclarationStatement>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, DeclarationStatement* statement) override;
	};

	class AssignmentStatementChecker : public StatementChecker<AssignmentStatement>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, AssignmentStatement* statement) override;
	};

	class ForStatementChecker : public StatementChecker<ForStatement>
	{
		void HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ForStatement* statement) override;
	};
}

#endif