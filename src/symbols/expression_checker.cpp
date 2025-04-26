#include "expression_checker.hpp"
#include "type_checker.hpp"

namespace HXSL
{
	std::unique_ptr<ExpressionChecker> ExpressionCheckerRegistry::handlers[NodeType_ExpressionCount];
	std::once_flag ExpressionCheckerRegistry::initFlag;

	void ExpressionCheckerRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<BinaryExpressionChecker, NodeType_BinaryExpression>();
				Register<UnaryExpressionChecker, NodeType_UnaryExpression>();
				Register<CastExpressionChecker, NodeType_CastExpression>();
				Register<ComplexMemberAccessExpressionChecker, NodeType_ComplexMemberAccessExpression>();
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

	void BinaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<BinaryExpression*>(node);

		auto left = expression->GetLeft().get();
		auto right = expression->GetRight().get();

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
			stack.push(right);
			stack.push(left);
		}
	}

	void UnaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<UnaryExpression*>(node);

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

	void CastExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<CastExpression*>(node);

		auto typeExpr = expression->GetTypeExpression().get();
		auto operand = expression->GetOperand().get();
		if (expression->GetLazyEvalState())
		{
			auto targetType = typeExpr->GetInferredType();
			auto operandType = operand->GetInferredType();

			if (targetType == nullptr || operandType == nullptr)
			{
				return;
			}

			SymbolDef* result;
			if (!checker.CastOperatorCheck(expression, typeExpr, operand, result, true))
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
			stack.push(typeExpr);
			stack.push(operand);
		}
	}

	void ComplexMemberAccessExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<ComplexMemberAccessExpression*>(node);

		auto left = expression->GetLeft().get();
		auto right = expression->GetRight().get();

		auto& state = expression->GetLazyEvalState();
		if (state == 1)
		{
			auto leftType = left->GetInferredType();

			if (!leftType)
			{
				return;
			}

			if (right->GetType() != NodeType_ComplexMemberAccessExpression)
			{
				expression->GetSymbolRef()->SetDeclaration(leftType);
				resolver.ResolveComplexMember(expression);
			}

			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(right);
		}
		else if (state == 2)
		{
			expression->SetInferredType(right->GetInferredType());
			expression->SetTraits(right->GetTraits());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(left);
		}
	}

	void MemberAccessExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<MemberAccessExpression*>(node);

		if (expression->GetLazyEvalState())
		{
			auto& nextExpr = expression->GetExpression();
			expression->SetInferredType(nextExpr->GetInferredType());
			expression->SetTraits(nextExpr->GetTraits());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			stack.push(expression->GetExpression().get());
		}
	}

	void MemberReferenceExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<MemberReferenceExpression*>(node);
		auto decl = expression->GetSymbolRef()->GetBaseDeclaration();
		expression->SetInferredType(decl);
		expression->SetTraits(ExpressionTraits(ExpressionTraitFlags_Mutable));
	}

	void FunctionCallExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<FunctionCallExpression*>(node);

		if (expression->GetLazyEvalState())
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

			if (success)
			{
				auto function = dynamic_cast<FunctionOverload*>(ref->GetDeclaration());
				HXSL_ASSERT(function, "Declaration in function call expression was not a function, this should never happen.");
				expression->SetInferredType(function->GetReturnSymbolRef().get()->GetDeclaration());
			}
			else
			{
				analyzer.LogError("Couldn't find an matching overload for '%s'.", expression->GetSpan(), signature.c_str());
			}
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

	void TernaryExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<TernaryExpression*>(node);

		auto condition = expression->GetCondition().get();
		auto trueBranch = expression->GetTrueBranch().get();
		auto falseBranch = expression->GetFalseBranch().get();

		if (expression->GetLazyEvalState())
		{
			auto conditionType = condition->GetInferredType();
			if (conditionType == nullptr)
			{
				return;
			}

			if (!checker.IsBooleanType(conditionType))
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

			if (!checker.AreTypesCompatible(trueBranchType, falseBranchType))
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
			stack.push(falseBranch);
			stack.push(trueBranch);
			stack.push(condition);
		}
	}

	void LiteralExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<LiteralExpression*>(node);
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

	void PrefixPostfixExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<UnaryExpression*>(node);

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

	void IndexerExpressionChecker::HandleExpression(Analyzer& analyzer, TypeChecker& checker, SymbolResolver& resolver, Expression* node, std::stack<Expression*>& stack)
	{
		auto expression = dynamic_cast<IndexerAccessExpression*>(node);

		auto& indices = expression->GetIndices();

		if (expression->GetLazyEvalState())
		{
			auto type = expression->GetSymbolRef()->GetBaseDeclaration();

			if (type == nullptr)
			{
				return;
			}

			if (type->GetType() != NodeType_Array)
			{
				analyzer.LogError("Type must be an array type to apply an indexer operator", expression->GetSpan());
				return;
			}

			auto array = dynamic_cast<Array*>(type);
			HXSL_ASSERT(array, "Array was null, this should never happen.");

			auto arrayDims = array->GetArrayDims().size();
			auto accessedDims = indices.size();

			if (arrayDims < accessedDims)
			{
				analyzer.LogError("Type must be an array type to apply an indexer operator", expression->GetSpan());
				return;
			}

			expression->SetInferredType(array->GetElementType());
		}
		else
		{
			expression->IncrementLazyEvalState();
			stack.push(expression);
			for (auto it = indices.rbegin(); it != indices.rend(); ++it)
			{
				stack.push(it->get());
			}
		}
	}
}