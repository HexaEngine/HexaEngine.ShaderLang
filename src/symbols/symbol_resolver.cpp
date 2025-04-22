#include "symbol_resolver.hpp"

namespace HXSL
{
	bool SymbolResolver::SymbolTypeSanityCheck(SymbolMetadata* metadata, SymbolRef* ref) const
	{
		auto refType = ref->GetType();
		auto defType = metadata->symbolType;
		auto& span = ref->GetSpan();

		switch (refType)
		{
		case SymbolRefType_Unknown:
			analyzer.LogError("Symbol '%s' has an unknown reference type", span, span.toString().c_str());
			return false;

		case SymbolRefType_Namespace:
			if (defType != SymbolType_Namespace)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Function:
			if (defType != SymbolType_Function)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Constructor:
			if (defType != SymbolType_Constructor)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_FunctionOrConstructor:
			if (defType != SymbolType_Function && defType != SymbolType_Constructor)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Function' or 'Constructor' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Struct:
			if (defType != SymbolType_Struct && defType != SymbolType_Primitive)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Struct' or 'Primitive' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Enum:
			if (defType != SymbolType_Enum)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected an 'Enum' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Variable:
			if (defType != SymbolType_Field && defType != SymbolType_Parameter && defType != SymbolType_Variable)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Field', 'Parameter', or 'Variable' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Attribute:
			if (defType != SymbolType_Attribute)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected an 'AttributeDeclaration' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Member:
			if (defType != SymbolType_Field)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Field' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_AnyType:
			if (defType != SymbolType_Struct && defType != SymbolType_Primitive && defType != SymbolType_Class)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Struct', 'Primitive', or 'Class' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case SymbolRefType_Any:
			return true;
			break;

		default:
			HXSL_ASSERT(false, "Unexpected symbol reference type.");
			return false;
		}

		return true;
	}

	bool SymbolResolver::SymbolVisibilityChecks(SymbolMetadata* metadata, SymbolRef* ref, ResolverScopeContext& context) const
	{
		auto& access = metadata->accessModifier;
		if (access == AccessModifier_Public) return true;

		auto decl = metadata->declaration;
		switch (access)
		{
		case AccessModifier_Private:
			break;
		case AccessModifier_Protected:
			HXSL_ASSERT(false, "Access modifier protected is not supported.");
			break;
		case AccessModifier_ProtectedInternal:
			HXSL_ASSERT(false, "Access modifier protected internal is not supported.");
			break;
		case AccessModifier_Internal:
			break;
		case AccessModifier_Public:
			return true;
			break;
		}

		return true;
	}

	bool SymbolResolver::TryResolve(const SymbolTable* table, const TextSpan& name, size_t nodeIndex, const SymbolTable*& outTable, size_t& outNodeIndex, SymbolDef*& outDefinition) const
	{
		auto index = table->FindNodeIndexFullPath(name, nodeIndex);
		if (index != -1)
		{
			outTable = table;
			outNodeIndex = index;
			outDefinition = table->GetNode(index).Metadata->declaration;
			return true;
		}
		outTable = nullptr;
		outNodeIndex = 0;
		outDefinition = nullptr;
		return false;
	}

	bool SymbolResolver::TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut, SymbolDef*& outDefinition) const
	{
		for (auto& ref : references)
		{
			auto table = ref.TargetAssembly->GetSymbolTable();
			auto& index = ref.LookupIndex;
			if (TryResolve(table, name, index, outTable, nodeIndexOut, outDefinition))
			{
				return true;
			}
		}

		return false;
	}

	SymbolDef* SymbolResolver::ResolveSymbol(const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut) const
	{
		SymbolDef* def;
		if (TryResolve(primitives.GetSymbolTable(), name, 0, outTable, nodeIndexOut, def))
		{
			return def;
		}

		auto local = targetAssembly->GetSymbolTable();
		if (TryResolve(local, name, current.SymbolTableIndex, outTable, nodeIndexOut, def))
		{
			return def;
		}

		for (auto& it = stack.rbegin(); it != stack.rend(); ++it)
		{
			auto& scope = *it;
			if (TryResolve(local, name, scope.SymbolTableIndex, outTable, nodeIndexOut, def))
			{
				return def;
			}
		}

		auto currentNamespace = current.CurrentNamespace;
		if (TryResolveInAssemblies(currentNamespace->GetAssemblyReferences(), name, outTable, nodeIndexOut, def))
		{
			return def;
		}

		for (auto& us : currentNamespace->GetUsings())
		{
			if (us.IsAlias) continue;
			if (TryResolveInAssemblies(us.AssemblyReferences, name, outTable, nodeIndexOut, def))
			{
				return def;
			}
		}

		for (auto& us : compilation->GetUsings())
		{
			if (us.IsAlias) continue;
			if (TryResolveInAssemblies(us.AssemblyReferences, name, outTable, nodeIndexOut, def))
			{
				return def;
			}
		}

		analyzer.LogError("Symbol not found '%s'", name, name.toString().c_str());
		return nullptr;
	}

	bool SymbolResolver::ResolveSymbol(SymbolRef* ref) const
	{
		auto& name = ref->GetSpan();

		const SymbolTable* table;
		size_t index;
		auto def = ResolveSymbol(name, table, index);
		if (!def)
		{
			return false;
		}

		auto metadata = table->GetNode(index).Metadata.get();
		if (!SymbolTypeSanityCheck(metadata, ref))
		{
			return false;
		}
		ref->SetTable(table, index);

		return true;
	}

	void SymbolResolver::PushScope(ASTNode* parent, const TextSpan& span, bool external)
	{
		stack.push(current);
		current.Parent = parent;
		if (external)
		{
			bool found = false;
			for (auto& assembly : references.GetAssemblies())
			{
				auto table = assembly->GetSymbolTable();
				auto index = table->FindNodeIndexFullPath(span, current.SymbolTableIndex);
				if (index != -1)
				{
					current.SymbolTableIndex = index;
					found = true;
					break;
				}
			}

			HXSL_ASSERT(found, "Couldn't resolve external node.");
		}
		else
		{
			auto local = targetAssembly->GetSymbolTable();
			current.SymbolTableIndex = local->FindNodeIndexFullPath(span, current.SymbolTableIndex);
		}
		HXSL_ASSERT(current.SymbolTableIndex != -1, "Invalid index");
	}

	void SymbolResolver::VisitClose(ASTNode* node, size_t depth)
	{
		if (current.Parent != node) return;
		current = stack.top();
		stack.pop();
	}

	TraversalBehavior SymbolResolver::VisitExternal(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case NodeType_Namespace:
		{
			auto currentNamespace = current.CurrentNamespace = node->As<Namespace>();
			PushScope(node, currentNamespace->GetName(), true);
		}
		break;
		case NodeType_Struct:
			PushScope(node, node->As<Struct>()->GetName(), true);
			break;
		case NodeType_Field:
		{
			auto field = node->As<Field>();
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_Function:
		{
			auto function = node->As<Function>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			PushScope(node, function->GetName(), true);
		}
		break;
		case NodeType_Parameter:
		{
			auto parameter = node->As<Parameter>();
			auto& ref = parameter->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		}

		return TraversalBehavior_Keep;
	}

	bool SymbolResolver::UseBeforeDeclarationCheck(SymbolRef* ref, ASTNode* parent) const
	{
		auto decl = ref->GetDeclaration();
		if (decl)
		{
			auto declStmt = dynamic_cast<DeclarationStatement*>(decl);
			if (declStmt)
			{
				auto usageID = parent->GetID();
				auto declID = declStmt->GetID();
				if (usageID < declID)
				{
					auto refStr = ref->GetSpan().toString();
					analyzer.LogError("Use of variable '%s' before its declaration.", parent->GetSpan(), refStr.c_str());
					return false;
				}
			}
		}
		return true;
	}

	TraversalBehavior SymbolResolver::Visit(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case NodeType_Namespace:
		{
			auto currentNamespace = current.CurrentNamespace = node->As<Namespace>();
			PushScope(node, currentNamespace->GetName(), false);
		}
		break;
		case NodeType_Struct:
			PushScope(node, node->As<Struct>()->GetName());
			break;
		case NodeType_Field:
		{
			auto field = node->As<Field>();
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_Function:
		{
			auto function = node->As<Function>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			PushScope(node, function->GetName());
		}
		break;
		case NodeType_Parameter:
		{
			auto parameter = node->As<Parameter>();
			auto& ref = parameter->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_BlockStatement:
		{
			std::string temp = MakeScopeId(current.ScopeCounter++);
			PushScope(node, TextSpan(temp));
		}
		break;
		case NodeType_DeclarationStatement:
		{
			auto declStatement = node->As<DeclarationStatement>();
			auto& ref = declStatement->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_SymbolRefExpression:
		{
			auto symbolRefExpression = node->As<SymbolRefExpression>();
			auto& ref = symbolRefExpression->GetSymbolRef();
			ResolveSymbol(ref.get());
			UseBeforeDeclarationCheck(ref.get(), node);
		}
		break;
		case NodeType_AttributeDeclaration:
		{
			auto attr = node->As<AttributeDeclaration>();
			auto& ref = attr->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_MemberAccessExpression:
		{
			if (deferred)
			{
				current = context.current;
				stack = context.stack;
			}
			auto memberAccessExpr = node->As<MemberAccessExpression>();
			auto next = node;
			auto result = ResolveMember(memberAccessExpr, next);
			if (!deferred && result == TraversalBehavior_Defer)
			{
				context = ResolverDeferralContext(current, stack);
			}
			node = next;
			return result;
		}
		break;
		}

		return TraversalBehavior_Keep;
	}

	SymbolDef* SymbolResolver::GetNumericType(const NumberType& type) const
	{
		switch (type)
		{
		case NumberType_Char:
		case NumberType_SByte:
		case NumberType_Short:
		case NumberType_UShort:
		case NumberType_Int:
			return ResolveSymbol("int");
		case NumberType_UInt:
			return ResolveSymbol("uint");
		case NumberType_LongLong:
			return ResolveSymbol("int64_t");
		case NumberType_ULongLong:
			return ResolveSymbol("uint64_t");
		case NumberType_Half:
			return ResolveSymbol("half");
		case NumberType_Float:
			return ResolveSymbol("float");
		case NumberType_Double:
			return ResolveSymbol("double");
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

	void SymbolResolver::TypeCheckExpression(Expression* node)
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
					auto& op = expression->GetOperator();
					SymbolDef* result;
					if (!BinaryOpTypeCheck(op, left, right, result))
					{
						analyzer.LogError("Couldn't find operator '%s' for '%s' and '%s'", expression->GetSpan(), ToString(op).c_str(), left->GetInferredType()->GetName().toString().c_str(), right->GetInferredType()->GetName().toString().c_str());
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
					expression->SetInferredType(expression->GetExpression()->GetInferredType());
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
				if (auto field = dynamic_cast<Field*>(decl))
				{
					decl = field->GetSymbolRef()->GetDeclaration();
				}
				expression->SetInferredType(decl);
			}
			break;
			case NodeType_FunctionCallExpression:
			{
				auto expression = dynamic_cast<FunctionCallExpression*>(current);

				if (expression->GetLazyEval())
				{
					auto function = dynamic_cast<Function*>(expression->GetSymbolRef().get()->GetDeclaration());
					// TODO: Add parameter checking and overloads.
					expression->SetInferredType(function->GetReturnSymbolRef().get()->GetDeclaration());
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
					def = ResolveSymbol("string");
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
			}
			break;
			default:
				break;
			}
		}
	}

	void SymbolResolver::TypeCheckStatement(ASTNode*& node)
	{
		auto& type = node->GetType();
		switch (type)
		{
		case NodeType_DeclarationStatement:
		{
			auto statement = dynamic_cast<DeclarationStatement*>(node);
			auto varType = statement->GetSymbolRef().get()->GetDeclaration();
			auto expr = statement->GetInitializer().get();
			TypeCheckExpression(expr);
			auto type = expr->GetInferredType();
		}
		break;
		default:
			break;
		}
	}

	TraversalBehavior SymbolResolver::TypeChecksExpression(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();
		if (!IsStatementType(type)) return TraversalBehavior_Keep;

		TypeCheckStatement(node);

		return TraversalBehavior_Keep;
	}

	static SymbolRef* GetResolvedTypeFromDecl(SymbolRef* ref)
	{
		auto decl = ref->GetDeclaration();
		if (auto typed = dynamic_cast<IHasSymbolRef*>(decl))
		{
			return typed->GetSymbolRef().get();
		}
		return nullptr;
	}

	int SymbolResolver::ResolveMemberInner(SymbolRef* type, IHasSymbolRef* getter) const
	{
		auto refInner = getter->GetSymbolRef().get();

		auto table = type->GetTable();
		auto index = type->GetTableIndex();

		if (table == nullptr)
		{
			return 1;
		}

		auto indexNext = table->FindNodeIndexPart(refInner->GetSpan(), index);
		if (indexNext == -1)
		{
			auto metadata = type->GetMetadata();
			if (metadata && metadata->declaration->GetType() == NodeType_Primitive)
			{
				return swizzleManager->VerifySwizzle(metadata->declaration->As<Primitive>(), refInner);
			}
			return -1;
		}
		auto metaInner = table->GetNode(indexNext).Metadata.get();
		if (!SymbolTypeSanityCheck(metaInner, refInner))
		{
			return -1;
		}
		refInner->SetTable(table, indexNext);

		return 0;
	}

	TraversalBehavior SymbolResolver::ResolveMember(MemberAccessExpression* memberAccessExpr, ASTNode*& next) const
	{
		auto getter = dynamic_cast<IHasSymbolRef*>(memberAccessExpr);
		auto chain = dynamic_cast<IChainExpression*>(memberAccessExpr);
		auto& refRoot = memberAccessExpr->GetSymbolRef();
		if (!ResolveSymbol(refRoot.get()))
		{
			return TraversalBehavior_Keep;
		}

		UseBeforeDeclarationCheck(refRoot.get(), memberAccessExpr);

		while (chain)
		{
			auto ref = getter->GetSymbolRef().get();

			SymbolRef* type = GetResolvedTypeFromDecl(ref);
			if (!type)
			{
				analyzer.LogError("Couldn't resolve type of member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return TraversalBehavior_Keep;
			}

			next = chain->chainNext().get();
			chain = dynamic_cast<IChainExpression*>(next);
			getter = dynamic_cast<IHasSymbolRef*>(next);
			if (getter == nullptr)
			{
				analyzer.LogError("Couldn't resolve member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return TraversalBehavior_Keep;
			}

			auto result = ResolveMemberInner(type, getter);
			if (result == -1)
			{
				analyzer.LogError("Couldn't resolve member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return TraversalBehavior_Keep;
			}
			else if (result == 1)
			{
				next = memberAccessExpr;
				return TraversalBehavior_Defer;
			}
		}

		return TraversalBehavior_Keep;
	}
}