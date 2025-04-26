#include "type_checker.hpp"
#include "symbol_resolver.hpp"
#include "expression_checker.hpp"

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
		if (!found || !operatorDecl)
		{
			analyzer.LogError("Cannot apply operator '%s', no operation defined between types '%s' and '%s'.",
				binary->GetSpan(),
				ToString(op).c_str(),
				leftType->GetName().toString().c_str(),
				rightType->GetName().toString().c_str());
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
			analyzer.LogError("Cannot apply operator '%s', no operation defined for type '%s'.",
				unary->GetSpan(),
				ToString(op).c_str(),
				leftType->GetName().toString().c_str());
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
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");
		return true;
	}

	bool TypeChecker::AreTypesCompatible(SymbolDef* a, SymbolDef* b)
	{
		return a->IsEquivalentTo(b);
	}

	bool TypeChecker::IsBooleanType(SymbolDef* a)
	{
		return AreTypesCompatible(a, resolver.ResolvePrimitiveSymbol("bool"));
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
			auto handler = ExpressionCheckerRegistry::GetHandler(type);
			handler->HandleExpression(analyzer, *this, resolver, current, stack);
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