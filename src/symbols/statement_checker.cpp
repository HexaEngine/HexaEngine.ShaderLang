#include "statement_checker.hpp"
#include "type_checker.hpp"

namespace HXSL
{
	std::unique_ptr<StatementCheckerBase> StatementCheckerRegistry::handlers[NodeType_StatementCount];
	std::once_flag StatementCheckerRegistry::initFlag;

	void StatementCheckerRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<DummyStatementChecker, NodeType_BlockStatement>();
				Register<ReturnStatementChecker, NodeType_ReturnStatement>();
				Register<DeclarationStatementChecker, NodeType_DeclarationStatement>();
				Register<AssignmentStatementChecker, NodeType_AssignmentStatement>();
				Register<ConditionalStatementChecker, NodeType_ForStatement>();
				Register<ConditionalStatementChecker, NodeType_WhileStatement>();
				Register<ConditionalStatementChecker, NodeType_IfStatement>();
				Register<ConditionalStatementChecker, NodeType_ElseIfStatement>();
			});
	}

	static const std::unordered_set<NodeType> functionLikeTypes =
	{
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
	};

	void ReturnStatementChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ReturnStatement* statement)
	{
		auto func = statement->FindAncestor<FunctionOverload>(functionLikeTypes);

		SymbolDef* retType = func->GetReturnType();
		SymbolDef* exprType = statement->GetReturnValueExpression()->GetInferredType();

		if (retType == nullptr || exprType == nullptr)
		{
			return;
		}

		std::unique_ptr<Expression> expr = std::move(statement->DetachReturnValueExpression());
		if (!checker.AreTypesCompatible(expr, retType, exprType))
		{
			analyzer.Log(RETURN_TYPE_DOES_NOT_MATCH, statement->GetSpan(), exprType->ToString(), retType->ToString());
		}
		statement->SetReturnValueExpression(std::move(expr));
	}

	void DeclarationStatementChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, DeclarationStatement* statement)
	{
		auto& exprInit = statement->GetInitializer();
		if (!exprInit)
		{
			return;
		}

		auto initType = exprInit->GetInferredType();
		auto declType = statement->GetDeclaredType();

		if (initType == nullptr || declType == nullptr)
		{
			return;
		}

		std::unique_ptr<Expression> expr = std::move(statement->DetachInitializer());
		if (!checker.AreTypesCompatible(expr, declType, initType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), initType->ToString(), declType->ToString());
		}
		statement->SetInitializer(std::move(expr));
	}

	void AssignmentStatementChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, AssignmentStatement* statement)
	{
		auto exprType = statement->GetExpression()->GetInferredType();
		auto targetType = statement->GetTarget()->GetInferredType();

		if (exprType == nullptr || targetType == nullptr)
		{
			return;
		}

		std::unique_ptr<Expression> expr = std::move(statement->DetachExpression());
		if (!checker.AreTypesCompatible(expr, targetType, exprType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), targetType->ToString(), exprType->ToString());
		}
		statement->SetExpression(std::move(expr));
	}

	void ConditionalStatementChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ConditionalStatement* statement)
	{
		auto conditionType = statement->GetCondition()->GetInferredType();

		if (conditionType == nullptr)
		{
			return;
		}

		std::unique_ptr<Expression> expr = std::move(statement->DetachCondition());
		if (!checker.IsBooleanType(expr, conditionType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), conditionType->ToString(), "bool");
		}
		statement->SetCondition(std::move(expr));
	}
}