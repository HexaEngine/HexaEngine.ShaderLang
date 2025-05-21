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
				Register<DummyStatementChecker, NodeType_ElseStatement>();
				Register<DummyStatementChecker, NodeType_ContinueStatement>();
				Register<DummyStatementChecker, NodeType_BreakStatement>();
				Register<DummyStatementChecker, NodeType_DiscardStatement>();
				Register<DummyStatementChecker, NodeType_DefaultCaseStatement>();
				Register<DummyStatementChecker, NodeType_ExpressionStatement>();
				Register<DummyStatementChecker, NodeType_AssignmentStatement>();
				Register<DummyStatementChecker, NodeType_CompoundAssignmentStatement>();

				Register<ReturnStatementChecker, NodeType_ReturnStatement>();
				Register<DeclarationStatementChecker, NodeType_DeclarationStatement>();

				Register<ConditionalStatementChecker, NodeType_ForStatement>();
				Register<ConditionalStatementChecker, NodeType_WhileStatement>();
				Register<ConditionalStatementChecker, NodeType_DoWhileStatement>();
				Register<ConditionalStatementChecker, NodeType_IfStatement>();
				Register<ConditionalStatementChecker, NodeType_ElseIfStatement>();

				Register<SwitchStatementChecker, NodeType_SwitchStatement>();
				Register<CaseStatementChecker, NodeType_CaseStatement>();
			});
	}

	static const std::unordered_set<NodeType> functionLikeTypes =
	{
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
	};

	void ReturnStatementChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ReturnStatement* statement)
	{
		auto func = statement->FindAncestor<FunctionOverload>(functionLikeTypes);

		SymbolDef* retType = func->GetReturnType();
		Expression* retExpression = statement->GetReturnValueExpression().get();
		if (retType == nullptr || retExpression == nullptr)
		{
			return;
		}

		SymbolDef* exprType = retExpression->GetInferredType();

		if (retType == nullptr || exprType == nullptr)
		{
			return;
		}

		ast_ptr<Expression> expr = std::move(statement->DetachReturnValueExpression());
		if (!checker.AreTypesCompatible(expr, retType, exprType))
		{
			analyzer.Log(RETURN_TYPE_DOES_NOT_MATCH, statement->GetSpan(), exprType->ToString(), retType->ToString());
		}
		statement->SetReturnValueExpression(std::move(expr));
	}

	void DeclarationStatementChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, DeclarationStatement* statement)
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

		ast_ptr<Expression> expr = std::move(statement->DetachInitializer());
		if (!checker.AreTypesCompatible(expr, declType, initType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), initType->ToString(), declType->ToString());
		}
		statement->SetInitializer(std::move(expr));
	}

	void ConditionalStatementChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, ConditionalStatement* statement)
	{
		auto conditionType = statement->GetCondition()->GetInferredType();

		if (conditionType == nullptr)
		{
			return;
		}

		ast_ptr<Expression> expr = std::move(statement->DetachCondition());
		if (!checker.IsBooleanType(expr, conditionType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), conditionType->ToString(), "bool");
		}
		statement->SetCondition(std::move(expr));
	}

	void SwitchStatementChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, SwitchStatement* statement)
	{
		auto exprType = statement->GetExpression()->GetInferredType();

		if (exprType == nullptr)
		{
			return;
		}

		ast_ptr<Expression> expr = std::move(statement->DetachExpression());
		if (!checker.IsIndexerType(expr, exprType))
		{
			analyzer.Log(EXPR_MUST_BE_INTEGRAL, expr->GetSpan());
		}
		statement->SetExpression(std::move(expr));
	}

	void CaseStatementChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CaseStatement* statement)
	{
		auto& expr = statement->GetExpression();
		auto exprType = expr->GetInferredType();

		if (exprType == nullptr)
		{
			return;
		}

		auto& traits = expr->GetTraits();

		if (!traits.HasFlag(ExpressionTraitFlags_Constant))
		{
			analyzer.Log(EXPECTED_CONST_EXPR, expr->GetSpan());
			return;
		}

		auto switchStatement = statement->FindAncestor<SwitchStatement>(NodeType_SwitchStatement, 1);

		if (!switchStatement)
		{
			HXSL_ASSERT(false, "Couldn't find switch case statement parent switch statement, this should never happen.");
			return;
		}

		auto switchType = switchStatement->GetExpression()->GetInferredType();

		if (!exprType->IsEquivalentTo(switchType))
		{
			analyzer.Log(TYPE_CONVERSION_NOT_FOUND, expr->GetSpan(), exprType->ToString(), switchType->ToString());
		}
	}
}