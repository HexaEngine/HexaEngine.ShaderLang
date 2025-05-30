#include "expression_checker.hpp"
#include "type_checker.hpp"

namespace HXSL
{
	std::unique_ptr<ExpressionCheckerBase> ExpressionCheckerRegistry::handlers[NodeType_ExpressionCount];
	std::once_flag ExpressionCheckerRegistry::initFlag;

	void ExpressionCheckerRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<DummyExpressionChecker, NodeType_EmptyExpression>();
				Register<BinaryExpressionChecker, NodeType_BinaryExpression>();
				Register<UnaryExpressionChecker, NodeType_UnaryExpression>();
				Register<CastExpressionChecker, NodeType_CastExpression>();
				Register<MemberAccessExpressionChecker, NodeType_MemberAccessExpression>();
				Register<MemberReferenceExpressionChecker, NodeType_MemberReferenceExpression>();
				Register<FunctionCallExpressionChecker, NodeType_FunctionCallExpression>();
				Register<TernaryExpressionChecker, NodeType_TernaryExpression>();
				Register<LiteralExpressionChecker, NodeType_LiteralExpression>();
				Register<PrefixPostfixExpressionChecker, NodeType_PrefixExpression>();
				Register<PrefixPostfixExpressionChecker, NodeType_PostfixExpression>();
				Register<IndexerExpressionChecker, NodeType_IndexerAccessExpression>();
				Register<AssignmentChecker, NodeType_AssignmentExpression>();
				Register<CompoundAssignmentChecker, NodeType_CompoundAssignmentExpression>();
			});
	}

	void BinaryExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, BinaryExpression* expression, std::stack<Expression*>& stack)
	{
		auto& left = expression->GetLeftMut();
		auto& right = expression->GetRightMut();

		if (expression->GetLazyEvalState())
		{
			auto leftType = left->GetInferredType();
			auto rightType = right->GetInferredType();

			if (leftType == nullptr || rightType == nullptr)
			{
				return;
			}

			SymbolDef* result;
			if (!checker.BinaryOperatorCheck(expression, left, right, result))
			{
				return;
			}

			expression->SetInferredType(result);

			//if (left->GetTraits().IsConstant() && right->GetTraits().IsConstant())
			{
				//left.reset();
				//right.reset();
			}
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(right.get());
			stack.push(left.get());
		}
	}

	void UnaryExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack)
	{
		auto operand = expression->GetOperand().get();

		if (expression->GetLazyEvalState())
		{
			auto operandType = operand->GetInferredType();

			if (operandType == nullptr)
			{
				return;
			}

			SymbolDef* result;
			if (!checker.UnaryOperatorCheck(expression, operand, result))
			{
				return;
			}

			expression->SetInferredType(result);
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(operand);
		}
	}

	void CastExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CastExpression* expression, std::stack<Expression*>& stack)
	{
		auto operand = expression->GetOperand().get();
		if (expression->GetLazyEvalState())
		{
			auto targetType = expression->GetTypeSymbol()->GetDeclaration();
			auto operandType = operand->GetInferredType();

			if (targetType == nullptr || operandType == nullptr)
			{
				return;
			}

			SymbolDef* result;
			if (!checker.CastOperatorCheck(expression, targetType, operand, result, true))
			{
				analyzer.Log(CANNOT_CAST_FROM_TO, expression->GetSpan(), targetType->ToString(), operandType->ToString());
				return;
			}

			expression->SetInferredType(result);
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(operand);
		}
	}

	void MemberAccessExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberAccessExpression* expression, std::stack<Expression*>& stack)
	{
		if (expression->GetLazyEvalState())
		{
			auto& nextExpr = expression->GetNextExpression();
			expression->SetInferredType(nextExpr->GetInferredType());
			expression->SetTraits(nextExpr->GetTraits());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(expression->GetNextExpression().get());
		}
	}

	void MemberReferenceExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberReferenceExpression* expression, std::stack<Expression*>& stack)
	{
		auto& ref = expression->GetSymbolRef();
		if (!ref->IsResolved() && !ref->IsNotFound())
		{
			HXSL_ASSERT(false, "");
		}

		auto decl = ref->GetBaseDeclaration();
		expression->SetInferredType(decl);
		expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Mutable));
	}

	void FunctionCallExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, FunctionCallExpression* expression, std::stack<Expression*>& stack)
	{
		auto state = expression->GetLazyEvalState();
		if (state == 1)
		{
			if (!expression->CanBuildOverloadSignature())
			{
				return;
			}

			SymbolDef* overload;
			if (!resolver.ResolveCallable(expression, overload))
			{
				return;
			}

			SymbolDef* type;
			if (auto constructor = overload->As<ConstructorOverload>())
			{
				type = constructor->GetTargetType();
				expression->SetFunctionCallFlag(FunctionCallExpressionFlags::ConstructorCall, true);
			}
			else
			{
				auto function = overload->As<FunctionOverload>();
				if (function == nullptr)
				{
					HXSL_ASSERT(false, "Declaration in function call expression was not a function, this should never happen.");
					return;
				}
				type = function->GetReturnSymbolRef()->GetDeclaration();
			}

			auto next = expression->GetNextExpression().get();
			if (next)
			{
				next->GetSymbolRef()->SetDeclaration(type);
				resolver.ResolveMember(next);

				expression->IncrementLazyEvalState();
				stack.push(expression);
				stack.push(next);
			}
			else
			{
				expression->SetInferredType(type);
				expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_None));
			}
		}
		else if (state == 2)
		{
			auto next = expression->GetNextExpression().get();
			expression->SetInferredType(next->GetInferredType());
			expression->SetTraits(next->GetTraits());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			auto& parameters = expression->GetParameters();
			for (auto it = parameters.rbegin(); it != parameters.rend(); ++it)
			{
				stack.push(it->get()->GetExpression().get());
			}
		}
	}

	void TernaryExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, TernaryExpression* expression, std::stack<Expression*>& stack)
	{
		auto& condition = expression->GetConditionMut();
		auto& trueBranch = expression->GetTrueBranchMut();
		auto& falseBranch = expression->GetFalseBranchMut();

		if (expression->GetLazyEvalState())
		{
			auto conditionType = condition->GetInferredType();
			if (conditionType == nullptr)
			{
				return;
			}

			if (!checker.IsBooleanType(condition, conditionType))
			{
				analyzer.Log(EXPECTED_BOOL_EXPR, expression->GetSpan());
				return;
			}

			auto trueBranchType = trueBranch->GetInferredType();
			auto falseBranchType = falseBranch->GetInferredType();

			if (trueBranchType == nullptr || falseBranchType == nullptr)
			{
				return;
			}

			if (!checker.AreTypesCompatible(trueBranch, trueBranchType, falseBranch, falseBranchType))
			{
				analyzer.Log(OPERAND_TYPES_INCOMPATIBLE, expression->GetSpan(), trueBranchType->ToString(), falseBranchType->ToString());
				return;
			}

			expression->SetInferredType(trueBranchType);
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(falseBranch.get());
			stack.push(trueBranch.get());
			stack.push(condition.get());
		}
	}

	void LiteralExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, LiteralExpression* expression, std::stack<Expression*>& stack)
	{
		auto& token = expression->GetLiteral();

		SymbolDef* def = nullptr;
		if (token.isLiteral())
		{
			def = resolver.ResolvePrimitiveSymbol("string");
		}
		else if (token.isNumeric())
		{
			auto& numeric = token.Numeric;
			def = checker.GetNumericType(numeric.Kind);
		}
		else if (token.isBool())
		{
			def = checker.GetBoolType();
		}
		else
		{
			HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
		}

		expression->SetInferredType(def);
		expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Constant));
	}

	void PrefixPostfixExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack)
	{
		auto operand = expression->GetOperand().get();

		if (expression->GetLazyEvalState())
		{
			auto operandType = operand->GetInferredType();

			if (operandType == nullptr)
			{
				return;
			}

			auto op = expression->GetOperator();

			if (op == Operator_Increment || op == Operator_Decrement)
			{
				if (!operand->GetTraits().IsMutable())
				{
					analyzer.Log(EXPR_MUST_BE_MUTABLE, operand->GetSpan());
					return;
				}
			}

			SymbolDef* result;
			if (!checker.UnaryOperatorCheck(expression, operand, result))
			{
				return;
			}

			expression->SetInferredType(result);
			expression->SetTraits({});
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(operand);
		}
	}

	void IndexerExpressionChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, IndexerAccessExpression* expression, std::stack<Expression*>& stack)
	{
		auto& index = expression->GetIndexExpressionMut();

		auto state = expression->GetLazyEvalState();
		if (state == 1)
		{
			auto type = expression->GetSymbolRef()->GetAncestorDeclaration(SymbolType_Array);
			auto indexType = index->GetInferredType();

			if (type == nullptr || indexType == nullptr)
			{
				return;
			}

			if (!checker.IsIndexerType(index, indexType))
			{
				analyzer.Log(EXPR_MUST_BE_INTEGRAL, expression->GetSpan());
				return;
			}

			if (type->GetType() != NodeType_Array)
			{
				analyzer.Log(EXPR_MUST_BE_ARRAY, expression->GetSpan());
				return;
			}

			auto array = dynamic_cast<Array*>(type);
			HXSL_ASSERT(array, "Array was null, this should never happen.");

			auto next = expression->GetNextExpression().get();
			if (next)
			{
				next->GetSymbolRef()->SetDeclaration(array);
				resolver.ResolveMember(next);

				expression->IncrementLazyEvalState();
				stack.push(expression);
				stack.push(next);
			}
			else
			{
				expression->SetInferredType(array->GetElementType());
				expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Mutable));
			}
		}
		else if (state == 2)
		{
			auto next = expression->GetNextExpression().get();
			expression->SetInferredType(next->GetInferredType());
			expression->SetTraits(next->GetTraits());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(index.get());
		}
	}

	void AssignmentChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, AssignmentExpression* expression, std::stack<Expression*>& stack)
	{
		auto target = expression->GetTarget().get();
		auto& assignment = expression->GetExpressionMut();

		if (expression->GetLazyEvalState())
		{
			auto targetType = target->GetInferredType();
			auto assignmentType = assignment->GetInferredType();
			if (targetType == nullptr || assignmentType == nullptr)
			{
				return;
			}

			if (!checker.AreTypesCompatible(assignment, targetType, assignmentType))
			{
				analyzer.Log(TYPE_CONVERSION_NOT_FOUND, assignment->GetSpan(), assignmentType->ToString(), targetType->ToString());
				return;
			}

			expression->SetInferredType(targetType);
			expression->SetTraits({});
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(assignment.get());
			stack.push(target);
		}
	}

	void CompoundAssignmentChecker::HandleExpression(SemanticAnalyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CompoundAssignmentExpression* expression, std::stack<Expression*>& stack)
	{
		auto& target = expression->GetTarget();
		auto& assignment = expression->GetExpressionMut();

		if (expression->GetLazyEvalState())
		{
			auto targetType = target->GetInferredType();
			auto assignmentType = assignment->GetInferredType();
			if (targetType == nullptr || assignmentType == nullptr)
			{
				return;
			}

			SymbolDef* result;
			if (!checker.BinaryCompoundOperatorCheck(expression, target, assignment, result))
			{
				return;
			}

			if (!checker.AreTypesCompatible(assignment, targetType, result))
			{
				analyzer.Log(TYPE_CONVERSION_NOT_FOUND, assignment->GetSpan(), result->ToString(), targetType->ToString());
				return;
			}

			expression->SetInferredType(result);
			expression->SetTraits({});
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(assignment.get());
			stack.push(target.get());
		}
	}
}