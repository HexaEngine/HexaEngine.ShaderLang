#include "type_checker.hpp"
#include "symbol_resolver.hpp"

namespace HXSL
{
	TraversalBehavior TypeChecker::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto& type = node->GetType();
		if (IsStatementType(type))
		{
			TypeCheckStatement(node);
		}
		else if (type == NodeType_OperatorOverload)
		{
			auto n = dynamic_cast<OperatorOverload*>(node);
			auto op = n->GetOperator();
			if (Operators::IsLogicOperator(op))
			{
				auto returnType = n->GetReturnSymbolRef()->GetDeclaration();
				if (returnType && !returnType->IsEquivalentTo(resolver.ResolvePrimitiveSymbol("bool")))
				{
					analyzer.LogError("Operator overload for '%s' must return type 'bool'. Found '%s' instead.", n->GetSpan(), ToString(op).c_str(), returnType->GetName().toString().c_str());
				}
			}
		}

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
			return resolver.ResolvePrimitiveSymbol("int");
		case NumberType_UInt:
			return resolver.ResolvePrimitiveSymbol("uint");
		case NumberType_LongLong:
			return resolver.ResolvePrimitiveSymbol("int64_t");
		case NumberType_ULongLong:
			return resolver.ResolvePrimitiveSymbol("uint64_t");
		case NumberType_Half:
			return resolver.ResolvePrimitiveSymbol("half");
		case NumberType_Float:
			return resolver.ResolvePrimitiveSymbol("float");
		case NumberType_Double:
			return resolver.ResolvePrimitiveSymbol("double");
		default:
			HXSL_ASSERT(false, "Unsupported numeric constant type.");
			return nullptr;
		}
	}

	bool TypeChecker::BinaryOperatorCheck(BinaryExpression* binary, const Expression* left, const Expression* right, SymbolDef*& result)
	{
		const Operator& op = binary->GetOperator();
		if (!Operators::IsValidOverloadOperator(op))
		{
			HXSL_ASSERT(result, "Operator was not an valid overload operator, this should never happen.");
			return false;
		}
		auto leftType = left->GetInferredType();
		auto rightType = right->GetInferredType();
		auto signature = binary->BuildOverloadSignature();

		auto& ref = binary->GetOperatorSymbolRef();

		bool found = false;

		{
			auto table = leftType->GetTable();
			auto& index = leftType->GetSymbolHandle();
			found = resolver.ResolveSymbol(ref.get(), signature, table, index, true);
		}

		if (!leftType->IsEquivalentTo(rightType))
		{
			auto table = rightType->GetTable();
			auto& index = rightType->GetSymbolHandle();
			if (resolver.ResolveSymbol(ref.get(), signature, table, index, true))
			{
				if (found)
				{
					analyzer.LogError("Ambiguous operator overload for '%s'.", binary->GetSpan(), signature.c_str());
					return false;
				}
				found = true;
			}
		}

		auto operatorDecl = dynamic_cast<OperatorOverload*>(ref->GetDeclaration());
		if (!operatorDecl || !found)
		{
			analyzer.LogError("No matching operator overload for '%s' found.", binary->GetSpan(), signature.c_str());
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");

		return true;
	}

	bool TypeChecker::UnaryOperatorCheck(UnaryExpression* unary, const Expression* operand, SymbolDef*& result)
	{
		const Operator& op = unary->GetOperator();
		if (!Operators::IsValidOverloadOperator(op))
		{
			HXSL_ASSERT(result, "Operator was not an valid overload operator, this should never happen.");
			return false;
		}
		auto leftType = operand->GetInferredType();
		auto signature = unary->BuildOverloadSignature();

		auto& ref = unary->GetOperatorSymbolRef();

		bool found = false;

		auto table = leftType->GetTable();
		auto& index = leftType->GetSymbolHandle();
		found = resolver.ResolveSymbol(ref.get(), signature, table, index, true);

		auto operatorDecl = dynamic_cast<OperatorOverload*>(ref->GetDeclaration());
		if (!operatorDecl || !found)
		{
			analyzer.LogError("No matching operator overload for '%s' found.", unary->GetSpan(), signature.c_str());
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");
		return true;
	}

	bool TypeChecker::CastOperatorCheck(CastExpression* cast, const Expression* typeExpr, const Expression* operand, SymbolDef*& result, bool explicitCast)
	{
		auto operandType = operand->GetInferredType();
		auto signature = cast->BuildOverloadSignature();

		auto& ref = cast->GetOperatorSymbolRef();

		bool found = false;

		auto table = operandType->GetTable();
		auto& index = operandType->GetSymbolHandle();
		found = resolver.ResolveSymbol(ref.get(), signature, table, index, true);

		auto operatorDecl = dynamic_cast<OperatorOverload*>(ref->GetDeclaration());
		if (!operatorDecl || !found)
		{
			analyzer.LogError("No matching operator overload for '%s' found.", cast->GetSpan(), signature.c_str());
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");
		return true;
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

				if (expression->GetLazyEvalState())
				{
					auto leftType = left->GetInferredType();
					auto rightType = right->GetInferredType();

					if (leftType == nullptr || rightType == nullptr)
					{
						break;
					}

					SymbolDef* result;
					if (!BinaryOperatorCheck(expression, left, right, result))
					{
						break;
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
			break;
			case NodeType_UnaryExpression:
			{
				auto expression = dynamic_cast<UnaryExpression*>(current);

				auto operand = expression->GetOperand().get();

				if (expression->GetLazyEvalState())
				{
					auto operandType = operand->GetInferredType();

					if (operandType == nullptr)
					{
						break;
					}

					SymbolDef* result;
					if (!UnaryOperatorCheck(expression, operand, result))
					{
						break;
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
			break;
			case NodeType_CastExpression:
			{
				auto expression = dynamic_cast<CastExpression*>(current);

				auto typeExpr = expression->GetTypeExpression().get();
				auto operand = expression->GetOperand().get();
				if (expression->GetLazyEvalState())
				{
					auto targetType = typeExpr->GetInferredType();
					auto operandType = operand->GetInferredType();

					if (targetType == nullptr || operandType == nullptr)
					{
						break;
					}
				}
				else
				{
					expression->IncrementLazyEvalState();
					stack.push(expression);
					stack.push(typeExpr);
					stack.push(operand);
				}
			}
			break;
			case NodeType_ComplexMemberAccessExpression:
			{
				auto expression = dynamic_cast<ComplexMemberAccessExpression*>(current);

				auto left = expression->GetLeft().get();
				auto right = expression->GetRight().get();

				auto& state = expression->GetLazyEvalState();
				if (state == 1)
				{
					auto leftType = left->GetInferredType();

					if (!leftType)
					{
						break;
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
			break;
			case NodeType_MemberAccessExpression:
			{
				auto expression = dynamic_cast<MemberAccessExpression*>(current);

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
			break;
			case NodeType_MemberReferenceExpression:
			{
				auto expression = dynamic_cast<MemberReferenceExpression*>(current);
				auto decl = expression->GetSymbolRef()->GetBaseDeclaration();
				expression->SetInferredType(decl);
			}
			break;
			case NodeType_FunctionCallExpression:
			{
				auto expression = dynamic_cast<FunctionCallExpression*>(current);

				if (expression->GetLazyEvalState())
				{
					if (!expression->CanBuildOverloadSignature())
					{
						break;
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
			break;
			case NodeType_LiteralExpression:
			{
				auto expression = dynamic_cast<LiteralExpression*>(current);
				auto& token = expression->GetLiteral();

				SymbolDef* def = nullptr;
				if (token.isLiteral())
				{
					def = resolver.ResolvePrimitiveSymbol("string");
				}
				else if (token.isNumeric())
				{
					auto& numeric = token.Numeric;
					def = GetNumericType(numeric.Kind);
				}
				else
				{
					HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
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