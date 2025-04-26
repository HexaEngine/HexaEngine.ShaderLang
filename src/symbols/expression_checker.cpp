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
			});
	}

	void BinaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, BinaryExpression* expression, std::stack<Expression*>& stack)
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
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(right.get());
			stack.push(left.get());
		}
	}

	void UnaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack)
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

	void CastExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, CastExpression* expression, std::stack<Expression*>& stack)
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
				analyzer.LogError("Cannot cast from '%s' to '%s' no cast operator defined.", expression->GetSpan(), targetType->GetName().toString().c_str(), operandType->GetName().toString().c_str());
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

	void MemberAccessExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberAccessExpression* expression, std::stack<Expression*>& stack)
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

	void MemberReferenceExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, MemberReferenceExpression* expression, std::stack<Expression*>& stack)
	{
		auto& ref = expression->GetSymbolRef();
		HXSL_ASSERT(ref->IsResolved(), "");
		auto decl = ref->GetBaseDeclaration();
		expression->SetInferredType(decl);
		expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Mutable));
	}

	void FunctionCallExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, FunctionCallExpression* expression, std::stack<Expression*>& stack)
	{
		auto state = expression->GetLazyEvalState();
		if (state == 1)
		{
			if (!expression->CanBuildOverloadSignature())
			{
				return;
			}

			auto& ref = expression->GetSymbolRef();
			auto signature = expression->BuildOverloadSignature();

			bool success = false;
			if (auto expr = expression->FindAncestor<MemberAccessExpression>(NodeType_MemberAccessExpression, 1))
			{
				auto memberRef = expr->GetSymbolRef()->GetBaseDeclaration();
				if (memberRef)
				{
					success = resolver.ResolveSymbol(ref.get(), TextSpan(signature), memberRef->GetTable(), memberRef->GetSymbolHandle());
				}
			}
			else
			{
				success = resolver.ResolveSymbol(ref.get(), TextSpan(signature));
			}

			if (!success)
			{
				analyzer.LogError("Couldn't find an matching overload for '%s'.", expression->GetSpan(), signature.c_str());
				return;
			}

			auto function = dynamic_cast<FunctionOverload*>(ref->GetDeclaration());
			HXSL_ASSERT(function, "Declaration in function call expression was not a function, this should never happen.");

			auto type = function->GetReturnSymbolRef()->GetDeclaration();
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

	void TernaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, TernaryExpression* expression, std::stack<Expression*>& stack)
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
				analyzer.LogError("Condition in ternary expression must be of type boolean.", expression->GetSpan());
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
				analyzer.LogError("Branches of ternary expression must have compatible types, found: %s and %s", expression->GetSpan(), trueBranchType->GetName().toString().c_str(), falseBranchType->GetName().toString().c_str());
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

	void LiteralExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, LiteralExpression* expression, std::stack<Expression*>& stack)
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
		else
		{
			HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
		}

		expression->SetInferredType(def);
		expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Constant));
	}

	void PrefixPostfixExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, UnaryExpression* expression, std::stack<Expression*>& stack)
	{
		auto operand = expression->GetOperand().get();

		if (expression->GetLazyEvalState())
		{
			auto operandType = operand->GetInferredType();

			if (operandType == nullptr)
			{
				return;
			}

			if (!operand->GetTraits().IsMutable())
			{
				analyzer.LogError("Cannot modify an immutable operand in a prefix expression.", operand->GetSpan());
				return;
			}

			auto op = expression->GetOperator();

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

	void IndexerExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, IndexerAccessExpression* expression, std::stack<Expression*>& stack)
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
				analyzer.LogError("Cannot convert '%s' to an indexer type. Only integer types are allowed for indexing.", expression->GetSpan(), indexType->GetName().toString().c_str());
				return;
			}

			if (type->GetType() != NodeType_Array)
			{
				analyzer.LogError("Tried to index into type '%s', which is not an array.", expression->GetSpan(), type->GetName().toString().c_str());
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
}