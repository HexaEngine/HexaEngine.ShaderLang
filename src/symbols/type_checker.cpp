#include "type_checker.hpp"
#include "symbol_resolver.hpp"

namespace HXSL
{
	TraversalBehavior TypeChecker::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto& type = node->GetType();
		if (!IsStatementType(type)) return TraversalBehavior_Keep;

		TypeCheckStatement(node);

		return TraversalBehavior_Keep;
	}

	void TypeChecker::VisitClose(ASTNode* node, size_t depth)
	{
	}

	SymbolDef* TypeChecker::GetNumericType(const NumberType& type) const
	{
		switch (type)
		{
		case NumberType_Char:
		case NumberType_SByte:
		case NumberType_Short:
		case NumberType_UShort:
		case NumberType_Int:
			return resolver.ResolveSymbol("int");
		case NumberType_UInt:
			return resolver.ResolveSymbol("uint");
		case NumberType_LongLong:
			return resolver.ResolveSymbol("int64_t");
		case NumberType_ULongLong:
			return resolver.ResolveSymbol("uint64_t");
		case NumberType_Half:
			return resolver.ResolveSymbol("half");
		case NumberType_Float:
			return resolver.ResolveSymbol("float");
		case NumberType_Double:
			return resolver.ResolveSymbol("double");
		default:
			HXSL_ASSERT(false, "Unsupported numeric constant type.");
			return nullptr;
		}
	}

	static bool TypeCheckPrim(Primitive* left, Primitive* right, Primitive*& out)
	{
		auto& leftType = left->GetKind();
		auto& rightType = right->GetKind();

		if (leftType == rightType)
		{
			out = left;
			return true;
		}

		if (leftType == PrimitiveKind_Float && rightType == PrimitiveKind_Int)
		{
			out = left;
			return true;
		}
		if (leftType == PrimitiveKind_Int && rightType == PrimitiveKind_Float)
		{
			out = right;
			return true;
		}

		return false;
	}

	static bool BinaryOpTypeCheck(const Operator& op, const Expression* left, const Expression* right, SymbolDef*& result)
	{
		auto leftType = left->GetInferredType();
		auto rightType = right->GetInferredType();

		// currently only primitives are allowed.
		if (leftType->GetSymbolType() != SymbolType_Primitive || rightType->GetSymbolType() != SymbolType_Primitive)
		{
			return false;
		}

		auto primLeft = dynamic_cast<Primitive*>(leftType);
		auto primRight = dynamic_cast<Primitive*>(rightType);

		Primitive* primOut;
		if (!TypeCheckPrim(primLeft, primRight, primOut))
		{
			return false;
		}
		result = primOut;

		auto& leftClass = primLeft->GetClass();
		auto& rightClass = primRight->GetClass();
		if (leftClass != rightClass || leftClass == PrimitiveClass_Matrix)
		{
			return false;
		}

		auto& outKind = primOut->GetKind();

		switch (op)
		{
		case Operator_Add:
		case Operator_Subtract:
		case Operator_Multiply:
		case Operator_Divide:
		case Operator_Modulus:
			return outKind != PrimitiveKind_Bool && outKind != PrimitiveKind_Void;

		case Operator_PlusAssign:
			break;
		case Operator_MinusAssign:
			break;
		case Operator_MultiplyAssign:
			break;
		case Operator_DivideAssign:
			break;
		case Operator_ModulusAssign:
			break;
		case Operator_BitwiseNot:
			break;
		case Operator_BitwiseShiftLeft:
			break;
		case Operator_BitwiseShiftRight:
			break;
		case Operator_BitwiseAnd:
			break;
		case Operator_BitwiseOr:
			break;
		case Operator_BitwiseXor:
			break;
		case Operator_BitwiseShiftLeftAssign:
			break;
		case Operator_BitwiseShiftRightAssign:
			break;
		case Operator_BitwiseAndAssign:
			break;
		case Operator_BitwiseOrAssign:
			break;
		case Operator_BitwiseXorAssign:
			break;
		case Operator_AndAnd:
			break;
		case Operator_OrOr:
			break;
		case Operator_LessThan:
			break;
		case Operator_GreaterThan:
			break;
		case Operator_Equal:
			break;
		case Operator_NotEqual:
			break;
		case Operator_LessThanOrEqual:
			break;
		case Operator_GreaterThanOrEqual:
			break;
		case Operator_Increment:
			break;
		case Operator_Decrement:
			break;
		case Operator_LogicalNot:
			break;

		default:
			break;
		}

		return false;
	}

	void TypeChecker::TypeCheckExpression(Expression* node)
	{
		std::stack<Expression*> stack;
		stack.push(node);

		while (!stack.empty())
		{
			Expression* current = stack.top();
			stack.pop();

			auto& type = current->GetType();
			switch (type)
			{
			case NodeType_BinaryExpression:
			{
				auto expression = dynamic_cast<BinaryExpression*>(current);

				auto left = expression->GetLeft().get();
				auto right = expression->GetRight().get();

				if (expression->GetLazyEval())
				{
					auto leftType = left->GetInferredType();
					auto rightType = right->GetInferredType();

					if (leftType == nullptr || rightType == nullptr)
					{
						break;
					}

					auto& op = expression->GetOperator();
					SymbolDef* result;
					if (!BinaryOpTypeCheck(op, left, right, result))
					{
						analyzer.LogError("Couldn't find operator '%s' for '%s' and '%s'", expression->GetSpan(), ToString(op).c_str(), leftType->GetName().toString().c_str(), rightType->GetName().toString().c_str());
						break;
					}

					expression->SetInferredType(result);
				}
				else
				{
					expression->SetLazyEval(true);
					stack.push(expression);
					stack.push(right);
					stack.push(left);
				}
			}
			break;
			case NodeType_UnaryExpression:
			{
				auto expression = dynamic_cast<UnaryExpression*>(current);

				if (expression->GetLazyEval())
				{
					// TODO: Add type checking here.
					expression->SetInferredType(expression->GetOperand()->GetInferredType());
				}
				else
				{
					expression->SetLazyEval(true);
					stack.push(expression);
					stack.push(expression->GetOperand().get());
				}
			}
			break;
			case NodeType_MemberAccessExpression:
			{
				auto expression = dynamic_cast<MemberAccessExpression*>(current);

				if (expression->GetLazyEval())
				{
					auto& nextExpr = expression->GetExpression();
					expression->SetInferredType(nextExpr->GetInferredType());
					expression->SetTraits(nextExpr->GetTraits());
				}
				else
				{
					expression->SetLazyEval(true);
					stack.push(expression);
					stack.push(expression->GetExpression().get());
				}
			}
			break;
			case NodeType_SymbolRefExpression:
			{
				auto expression = dynamic_cast<SymbolRefExpression*>(current);
				auto decl = expression->GetSymbolRef()->GetDeclaration();
				if (auto getter = dynamic_cast<IHasSymbolRef*>(decl))
				{
					decl = getter->GetSymbolRef()->GetDeclaration();
				}
				expression->SetInferredType(decl);
			}
			break;
			case NodeType_FunctionCallExpression:
			{
				auto expression = dynamic_cast<FunctionCallExpression*>(current);

				if (expression->GetLazyEval())
				{
					auto& ref = expression->GetSymbolRef();
					auto signature = expression->BuildOverloadSignature();

					bool success = false;
					if (auto expr = expression->FindAncestor<MemberAccessExpression>(NodeType_MemberAccessExpression, 1))
					{
						auto memberRef = expr->GetSymbolRef()->GetBaseDeclaration();
						if (memberRef)
						{
							success = resolver.ResolveSymbol(ref.get(), TextSpan(signature), memberRef->GetTable(), memberRef->GetTableIndex());
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
					expression->SetLazyEval(true);
					stack.push(expression);
					auto& parameters = expression->GetParameters();
					for (auto it = parameters.rbegin(); it != parameters.rend(); ++it)
					{
						stack.push(it->get()->GetExpression().get());
					}
				}
			}
			break;
			case NodeType_LiteralExpression:
			{
				auto expression = dynamic_cast<LiteralExpression*>(current);
				auto& token = expression->GetLiteral();

				SymbolDef* def = nullptr;
				if (token.isLiteral())
				{
					def = resolver.ResolveSymbol("string");
				}
				else if (token.isNumeric())
				{
					auto& numeric = token.Numeric;
					def = GetNumericType(numeric.Kind);
				}
				else
				{
					HXSL_ASSERT(false, "Invalid token as constant expression."); // the parser handles this already normally just add it as safe guard.
				}

				expression->SetInferredType(def);
				expression->SetTraits(ExpressionTraits(true));
			}
			break;
			default:
				HXSL_ASSERT(false, "Missing expression type checking case.");
				break;
			}
		}
	}

	static const std::unordered_set<NodeType> functionLikeTypes =
	{
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
	};

#pragma warning(push)
#pragma warning(disable: 28182) // node cannot be null.
#pragma warning(pop)
	void TypeChecker::TypeCheckStatement(ASTNode*& node)
	{
		auto& type = node->GetType();

		if (auto getter = dynamic_cast<IHasExpressions*>(node))
		{
			for (auto expr : getter->GetExpressions())
			{
				TypeCheckExpression(expr);
			}
		}

		switch (type)
		{
		case NodeType_ReturnStatement:
		{
			auto ret = dynamic_cast<ReturnStatement*>(node);
			auto ancestor = node->FindAncestors(functionLikeTypes);
			auto func = dynamic_cast<FunctionOverload*>(ancestor);
			HXSL_ASSERT(func, "Function-like ancestor of statement couldn't be found");

			SymbolDef* retType = func->GetReturnSymbolRef()->GetDeclaration();
			SymbolDef* exprType = ret->GetReturnValueExpression()->GetInferredType();

			if (retType && exprType && !retType->IsEquivalentTo(exprType))
			{
				analyzer.LogError("Return type mismatch: expected '%s' but got '%s'", ret->GetSpan(), retType->GetFullyQualifiedName().c_str(), exprType->GetFullyQualifiedName().c_str());
			}
		}
		break;
		case NodeType_IfStatement:
		{
			auto ifStmt = dynamic_cast<IfStatement*>(node);
		}
		break;

		default:
			break;
		}
	}
}