#include "type_checker.hpp"
#include "symbol_resolver.hpp"
#include "expression_checker.hpp"
#include "statement_checker.hpp"

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
					analyzer.Log(EXPECTED_BOOL_EXPR, n->GetSpan());
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
		case NumberType_UInt8:
		case NumberType_Int8:
		case NumberType_Int16:
		case NumberType_UInt16:
		case NumberType_Int32:
			return resolver.ResolvePrimitiveSymbol("int");
		case NumberType_UInt32:
			return resolver.ResolvePrimitiveSymbol("uint");
		case NumberType_Int64:
			return resolver.ResolvePrimitiveSymbol("int64_t");
		case NumberType_UInt64:
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

	bool TypeChecker::ImplicitBinaryOperatorCheck(SymbolRef* opRef, SymbolDef* a, SymbolDef* b, Operator op, OperatorOverload*& castMatchOut)
	{
		auto getter = dynamic_cast<IHasOperatorOverloads*>(a);
		if (!getter)
		{
			HXSL_ASSERT(false, "IHasOperators was null, this should never happen.");
			return false;
		}

		auto& handleB = b->GetSymbolHandle();
		auto lookup = ToLookupChar(Operator_Cast);

		std::string signature;
		BinaryExpression::PrepareOverloadSignature(signature, op);
		for (auto& _operator : getter->GetOperators())
		{
			if (_operator->GetName()[0] != lookup || (_operator->GetOperatorFlags() & OperatorFlags_Implicit) == 0)
			{
				continue;
			}

			auto decl = _operator->GetReturnType();
			BinaryExpression::BuildOverloadSignature(signature, decl, b);

			auto match = handleB.FindPart(signature);
			if (match.valid())
			{
				castMatchOut = _operator.get();
				opRef->SetTable(match);
				return true;
			}
		}

		return false;
	}

	static void InjectCast(std::unique_ptr<Expression>& target, OperatorOverload* cast, SymbolDef* targetType)
	{
		auto castExpr = std::make_unique<CastExpression>(TextSpan(), cast->MakeSymbolRef(), targetType->MakeSymbolRef(), nullptr);
		castExpr->SetInferredType(targetType);
		InjectNode(target, std::move(castExpr), &CastExpression::SetOperand);
	}

	bool TypeChecker::BinaryOperatorCheck(BinaryExpression* binary, std::unique_ptr<Expression>& left, std::unique_ptr<Expression>& right, SymbolDef*& result)
	{
		const Operator& op = binary->GetOperator();
		if (!Operators::IsValidOverloadOperator(op))
		{
			HXSL_ASSERT(false, "Operator was not an valid overload operator, this should never happen.");
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
					analyzer.Log(AMBIGUOUS_OP_OVERLOAD, binary->GetSpan(), signature);
					return false;
				}
				found = true;
			}
		}

		if (!found)
		{
			OperatorOverload* castMatch;
			if (ImplicitBinaryOperatorCheck(ref.get(), leftType, rightType, op, castMatch))
			{
				InjectCast(left, castMatch, rightType);
				found = true;
			}

			if (!found && ImplicitBinaryOperatorCheck(ref.get(), rightType, leftType, op, castMatch))
			{
				InjectCast(right, castMatch, leftType);
				found = true;
			}
		}

		auto operatorDecl = dynamic_cast<OperatorOverload*>(ref->GetDeclaration());
		if (!found || !operatorDecl)
		{
			analyzer.Log(OP_OVERLOAD_NOT_FOUND_BINARY,
				binary->GetSpan(),
				ToString(op),
				leftType->ToString(),
				rightType->ToString());
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");

		return true;
	}

	bool TypeChecker::BinaryCompoundOperatorCheck(CompoundAssignmentExpression* binary, const std::unique_ptr<Expression>& left, std::unique_ptr<Expression>& right, SymbolDef*& result)
	{
		const Operator& op = binary->GetOperator();
		if (!Operators::IsValidOverloadOperator(op))
		{
			HXSL_ASSERT(false, "Operator was not an valid overload operator, this should never happen.");
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
					analyzer.Log(AMBIGUOUS_OP_OVERLOAD, binary->GetSpan(), signature);
					return false;
				}
				found = true;
			}
		}

		if (!found)
		{
			OperatorOverload* castMatch;
			if (ImplicitBinaryOperatorCheck(ref.get(), rightType, leftType, op, castMatch))
			{
				InjectCast(right, castMatch, leftType);
				found = true;
			}
		}

		auto operatorDecl = dynamic_cast<OperatorOverload*>(ref->GetDeclaration());
		if (!found || !operatorDecl)
		{
			analyzer.Log(OP_OVERLOAD_NOT_FOUND_BINARY,
				binary->GetSpan(),
				ToString(op),
				leftType->ToString(),
				rightType->ToString());
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
			analyzer.Log(OP_OVERLOAD_NOT_FOUND_UNARY,
				unary->GetSpan(),
				ToString(op),
				leftType->ToString());
			return false;
		}

		result = operatorDecl->GetReturnSymbolRef()->GetDeclaration();

		HXSL_ASSERT(result, "Operator return type declaration was nullptr, this should never happen.");
		return true;
	}

	bool TypeChecker::CastOperatorCheck(CastExpression* cast, const SymbolDef* type, const Expression* operand, SymbolDef*& result, bool explicitCast)
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

	bool TypeChecker::CastOperatorCheck(const SymbolDef* target, const SymbolDef* source, std::unique_ptr<SymbolRef>& result)
	{
		auto signature = CastExpression::BuildOverloadSignature(target, source);

		auto ref = std::make_unique<SymbolRef>(signature, SymbolRefType_OperatorOverload, false);

		auto table = source->GetTable();
		auto& index = source->GetSymbolHandle();
		auto found = resolver.ResolveSymbol(ref.get(), signature, table, index, true);
		if (!found)
		{
			return false;
		}

		auto operatorDecl = static_cast<OperatorOverload*>(ref->GetDeclaration());

		if ((operatorDecl->GetOperatorFlags() & OperatorFlags_Implicit) == 0)
		{
			return false;
		}

		result = std::move(ref);

		return true;
	}

	// for unary-like operations.
	bool TypeChecker::AreTypesCompatible(std::unique_ptr<Expression>& insertPoint, SymbolDef* target, SymbolDef* source)
	{
		if (source->IsEquivalentTo(target))
		{
			return true;
		}

		// implicit cast handling.
		std::unique_ptr<SymbolRef> opRef;
		if (!CastOperatorCheck(target, source, opRef))
		{
			return false;
		}

		auto castExpr = std::make_unique<CastExpression>(TextSpan(), std::move(opRef), target->MakeSymbolRef(), nullptr);
		InjectNode(insertPoint, std::move(castExpr), &CastExpression::SetOperand);

		return true;
	}

	// for binary-like operations.
	bool TypeChecker::AreTypesCompatible(std::unique_ptr<Expression>& insertPointA, SymbolDef* a, std::unique_ptr<Expression>& insertPointB, SymbolDef* b)
	{
		if (a->IsEquivalentTo(b))
		{
			return true;
		}

		return false; // TODO: set me true later.
	}

	bool TypeChecker::IsBooleanType(std::unique_ptr<Expression>& insertPoint, SymbolDef* source)
	{
		return AreTypesCompatible(insertPoint, resolver.ResolvePrimitiveSymbol("bool"), source);
	}

	bool TypeChecker::IsIndexerType(std::unique_ptr<Expression>& insertPoint, SymbolDef* source)
	{
		return AreTypesCompatible(insertPoint, resolver.ResolvePrimitiveSymbol("int"), source) || AreTypesCompatible(insertPoint, resolver.ResolvePrimitiveSymbol("uint"), source);
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
			HXSL_ASSERT(handler, "ExpressionChecker handler was null, forgot to implement more?");
			if (handler)
			{
				handler->HandleExpression(analyzer, *this, resolver, current, stack);
			}
		}
	}

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

		if (auto statement = dynamic_cast<Statement*>(node))
		{
			auto handler = StatementCheckerRegistry::GetHandler(type);
			HXSL_ASSERT(handler, "StatementChecker handler was null, forgot to implement more?");
			if (handler)
			{
				handler->HandleExpression(analyzer, *this, resolver, statement);
			}
		}
	}
}