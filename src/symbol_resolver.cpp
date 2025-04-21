#include "symbol_resolver.hpp"

namespace HXSL
{
	bool HXSLSymbolResolver::SymbolTypeSanityCheck(SymbolMetadata* metadata, HXSLSymbolRef* ref) const
	{
		auto refType = ref->GetType();
		auto defType = metadata->SymbolType;
		auto& span = ref->GetSpan();

		switch (refType)
		{
		case HXSLSymbolRefType_Unknown:
			analyzer.LogError("Symbol '%s' has an unknown reference type", span, span.toString().c_str());
			return false;

		case HXSLSymbolRefType_Namespace:
			if (defType != HXSLSymbolType_Namespace)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Function:
			if (defType != HXSLSymbolType_Function)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Constructor:
			if (defType != HXSLSymbolType_Constructor)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a '%s' definition", span, span.toString().c_str(),
					ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_FunctionOrConstructor:
			if (defType != HXSLSymbolType_Function && defType != HXSLSymbolType_Constructor)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Function' or 'Constructor' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Struct:
			if (defType != HXSLSymbolType_Struct && defType != HXSLSymbolType_Primitive)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Struct' or 'Primitive' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Enum:
			if (defType != HXSLSymbolType_Enum)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected an 'Enum' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Variable:
			if (defType != HXSLSymbolType_Field && defType != HXSLSymbolType_Parameter && defType != HXSLSymbolType_Variable)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Field', 'Parameter', or 'Variable' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Attribute:
			if (defType != HXSLSymbolType_Attribute)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected an 'AttributeDeclaration' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Member:
			if (defType != HXSLSymbolType_Field)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Field' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_AnyType:
			if (defType != HXSLSymbolType_Struct && defType != HXSLSymbolType_Primitive && defType != HXSLSymbolType_Class)
			{
				analyzer.LogError("Symbol '%s' is of type '%s', but expected a 'Struct', 'Primitive', or 'Class' definition (got '%s')",
					span, span.toString().c_str(), ToString(defType).c_str());
				return false;
			}
			break;

		case HXSLSymbolRefType_Any:
			return true;
			break;

		default:
			HXSL_ASSERT(false, "Unexpected symbol reference type.");
			return false;
		}

		return true;
	}

	bool HXSLSymbolResolver::SymbolVisibilityChecks(SymbolMetadata* metadata, HXSLSymbolRef* ref, ResolverScopeContext& context) const
	{
		auto& access = metadata->AccessModifier;
		if (access == HXSLAccessModifier_Public) return true;

		auto decl = metadata->Declaration;
		switch (access)
		{
		case HXSLAccessModifier_Private:
			break;
		case HXSLAccessModifier_Protected:
			HXSL_ASSERT(false, "Access modifier protected is not supported.");
			break;
		case HXSLAccessModifier_ProtectedInternal:
			HXSL_ASSERT(false, "Access modifier protected internal is not supported.");
			break;
		case HXSLAccessModifier_Internal:
			break;
		case HXSLAccessModifier_Public:
			return true;
			break;
		}

		return true;
	}

	bool HXSLSymbolResolver::TryResolve(const SymbolTable* table, const TextSpan& name, size_t nodeIndex, const SymbolTable*& outTable, size_t& outNodeIndex, HXSLSymbolDef*& outDefinition) const
	{
		auto index = table->FindNodeIndexFullPath(name, nodeIndex);
		if (index != -1)
		{
			outTable = table;
			outNodeIndex = index;
			outDefinition = table->GetNode(index).Metadata->Declaration;
			return true;
		}
		outTable = nullptr;
		outNodeIndex = 0;
		outDefinition = nullptr;
		return false;
	}

	bool HXSLSymbolResolver::TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut, HXSLSymbolDef*& outDefinition) const
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

	HXSLSymbolDef* HXSLSymbolResolver::ResolveSymbol(const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut) const
	{
		HXSLSymbolDef* def;
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

	bool HXSLSymbolResolver::ResolveSymbol(HXSLSymbolRef* ref) const
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

	void HXSLSymbolResolver::PushScope(ASTNode* parent, const TextSpan& span, bool external)
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

	void HXSLSymbolResolver::VisitClose(ASTNode* node, size_t depth)
	{
		if (current.Parent != node) return;
		current = stack.top();
		stack.pop();
	}

	HXSLTraversalBehavior HXSLSymbolResolver::VisitExternal(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case HXSLNodeType_Namespace:
		{
			auto currentNamespace = current.CurrentNamespace = node->As<HXSLNamespace>();
			PushScope(node, currentNamespace->GetName(), true);
		}
		break;
		case HXSLNodeType_Struct:
			PushScope(node, node->As<HXSLStruct>()->GetName(), true);
			break;
		case HXSLNodeType_Field:
		{
			auto field = node->As<HXSLField>();
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case HXSLNodeType_Function:
		{
			auto function = node->As<HXSLFunction>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			PushScope(node, function->GetName(), true);
		}
		break;
		case HXSLNodeType_Parameter:
		{
			auto parameter = node->As<HXSLParameter>();
			auto& ref = parameter->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		}

		return HXSLTraversalBehavior_Keep;
	}

	bool HXSLSymbolResolver::UseBeforeDeclarationCheck(HXSLSymbolRef* ref, ASTNode* parent) const
	{
		auto decl = ref->GetDeclaration();
		if (decl)
		{
			auto declStmt = dynamic_cast<HXSLDeclarationStatement*>(decl);
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

	HXSLTraversalBehavior HXSLSymbolResolver::Visit(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case HXSLNodeType_Namespace:
		{
			auto currentNamespace = current.CurrentNamespace = node->As<HXSLNamespace>();
			PushScope(node, currentNamespace->GetName(), false);
		}
		break;
		case HXSLNodeType_Struct:
			PushScope(node, node->As<HXSLStruct>()->GetName());
			break;
		case HXSLNodeType_Field:
		{
			auto field = node->As<HXSLField>();
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case HXSLNodeType_Function:
		{
			auto function = node->As<HXSLFunction>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			PushScope(node, function->GetName());
		}
		break;
		case HXSLNodeType_Parameter:
		{
			auto parameter = node->As<HXSLParameter>();
			auto& ref = parameter->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case HXSLNodeType_BlockStatement:
		{
			std::string temp = MakeScopeId(current.ScopeCounter++);
			PushScope(node, TextSpan(temp));
		}
		break;
		case HXSLNodeType_DeclarationStatement:
		{
			auto declStatement = node->As<HXSLDeclarationStatement>();
			auto& ref = declStatement->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case HXSLNodeType_SymbolRefExpression:
		{
			auto symbolRefExpression = node->As<HXSLSymbolRefExpression>();
			auto& ref = symbolRefExpression->GetSymbolRef();
			ResolveSymbol(ref.get());
			UseBeforeDeclarationCheck(ref.get(), node);
		}
		break;
		case HXSLNodeType_AttributeDeclaration:
		{
			auto attr = node->As<HXSLAttributeDeclaration>();
			auto& ref = attr->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case HXSLNodeType_MemberAccessExpression:
		{
			if (deferred)
			{
				current = context.current;
				stack = context.stack;
			}
			auto memberAccessExpr = node->As<HXSLMemberAccessExpression>();
			auto next = node;
			auto result = ResolveMember(memberAccessExpr, next);
			if (!deferred && result == HXSLTraversalBehavior_Defer)
			{
				context = ResolverDeferralContext(current, stack);
			}
			node = next;
			return result;
		}
		break;
		}

		return HXSLTraversalBehavior_Keep;
	}

	HXSLSymbolDef* HXSLSymbolResolver::GetNumericType(const HXSLNumberType& type) const
	{
		switch (type)
		{
		case HXSLNumberType_Char:
		case HXSLNumberType_SByte:
		case HXSLNumberType_Short:
		case HXSLNumberType_UShort:
		case HXSLNumberType_Int:
			return ResolveSymbol("int");
		case HXSLNumberType_UInt:
			return ResolveSymbol("uint");
		case HXSLNumberType_LongLong:
			return ResolveSymbol("int64_t");
		case HXSLNumberType_ULongLong:
			return ResolveSymbol("uint64_t");
		case HXSLNumberType_Half:
			return ResolveSymbol("half");
		case HXSLNumberType_Float:
			return ResolveSymbol("float");
		case HXSLNumberType_Double:
			return ResolveSymbol("double");
		default:
			HXSL_ASSERT(false, "Unsupported numeric constant type.");
			return nullptr;
		}
	}

	static bool TypeCheckPrim(HXSLPrimitive* left, HXSLPrimitive* right, HXSLPrimitive*& out)
	{
		auto& leftType = left->GetKind();
		auto& rightType = right->GetKind();

		if (leftType == rightType)
		{
			out = left;
			return true;
		}

		if (leftType == HXSLPrimitiveKind_Float && rightType == HXSLPrimitiveKind_Int)
		{
			out = left;
			return true;
		}
		if (leftType == HXSLPrimitiveKind_Int && rightType == HXSLPrimitiveKind_Float)
		{
			out = right;
			return true;
		}

		return false;
	}

	static bool BinaryOpTypeCheck(const HXSLOperator& op, const HXSLExpression* left, const HXSLExpression* right, HXSLSymbolDef*& result)
	{
		auto leftType = left->GetInferredType();
		auto rightType = right->GetInferredType();

		// currently only primitives are allowed.
		if (leftType->GetSymbolType() != HXSLSymbolType_Primitive || rightType->GetSymbolType() != HXSLSymbolType_Primitive)
		{
			return false;
		}

		auto primLeft = dynamic_cast<HXSLPrimitive*>(leftType);
		auto primRight = dynamic_cast<HXSLPrimitive*>(rightType);

		HXSLPrimitive* primOut;
		if (!TypeCheckPrim(primLeft, primRight, primOut))
		{
			return false;
		}
		result = primOut;

		auto& leftClass = primLeft->GetClass();
		auto& rightClass = primRight->GetClass();
		if (leftClass != rightClass || leftClass == HXSLPrimitiveClass_Matrix)
		{
			return false;
		}

		auto& outKind = primOut->GetKind();

		switch (op)
		{
		case HXSLOperator_Add:
		case HXSLOperator_Subtract:
		case HXSLOperator_Multiply:
		case HXSLOperator_Divide:
		case HXSLOperator_Modulus:
			return outKind != HXSLPrimitiveKind_Bool && outKind != HXSLPrimitiveKind_Void;

		case HXSLOperator_PlusAssign:
			break;
		case HXSLOperator_MinusAssign:
			break;
		case HXSLOperator_MultiplyAssign:
			break;
		case HXSLOperator_DivideAssign:
			break;
		case HXSLOperator_ModulusAssign:
			break;
		case HXSLOperator_BitwiseNot:
			break;
		case HXSLOperator_BitwiseShiftLeft:
			break;
		case HXSLOperator_BitwiseShiftRight:
			break;
		case HXSLOperator_BitwiseAnd:
			break;
		case HXSLOperator_BitwiseOr:
			break;
		case HXSLOperator_BitwiseXor:
			break;
		case HXSLOperator_BitwiseShiftLeftAssign:
			break;
		case HXSLOperator_BitwiseShiftRightAssign:
			break;
		case HXSLOperator_BitwiseAndAssign:
			break;
		case HXSLOperator_BitwiseOrAssign:
			break;
		case HXSLOperator_BitwiseXorAssign:
			break;
		case HXSLOperator_AndAnd:
			break;
		case HXSLOperator_OrOr:
			break;
		case HXSLOperator_LessThan:
			break;
		case HXSLOperator_GreaterThan:
			break;
		case HXSLOperator_Equal:
			break;
		case HXSLOperator_NotEqual:
			break;
		case HXSLOperator_LessThanOrEqual:
			break;
		case HXSLOperator_GreaterThanOrEqual:
			break;
		case HXSLOperator_Increment:
			break;
		case HXSLOperator_Decrement:
			break;
		case HXSLOperator_LogicalNot:
			break;

		default:
			break;
		}

		return false;
	}

	void HXSLSymbolResolver::TypeCheckExpression(HXSLExpression* node)
	{
		std::stack<HXSLExpression*> stack;
		stack.push(node);

		while (!stack.empty())
		{
			HXSLExpression* current = stack.top();
			stack.pop();

			auto& type = current->GetType();
			switch (type)
			{
			case HXSLNodeType_BinaryExpression:
			{
				auto expression = dynamic_cast<HXSLBinaryExpression*>(current);

				auto left = expression->GetLeft().get();
				auto right = expression->GetRight().get();

				if (expression->GetLazyEval())
				{
					auto& op = expression->GetOperator();
					HXSLSymbolDef* result;
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
			case HXSLNodeType_UnaryExpression:
			{
				auto expression = dynamic_cast<HXSLUnaryExpression*>(current);

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
			case HXSLNodeType_MemberAccessExpression:
			{
				auto expression = dynamic_cast<HXSLMemberAccessExpression*>(current);

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
			case HXSLNodeType_SymbolRefExpression:
			{
				auto expression = dynamic_cast<HXSLSymbolRefExpression*>(current);
				auto decl = expression->GetSymbolRef()->GetDeclaration();
				if (auto field = dynamic_cast<HXSLField*>(decl))
				{
					decl = field->GetSymbolRef()->GetDeclaration();
				}
				expression->SetInferredType(decl);
			}
			break;
			case HXSLNodeType_FunctionCallExpression:
			{
				auto expression = dynamic_cast<HXSLFunctionCallExpression*>(current);

				if (expression->GetLazyEval())
				{
					auto function = dynamic_cast<HXSLFunction*>(expression->GetSymbolRef().get()->GetDeclaration());
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
			case HXSLNodeType_LiteralExpression:
			{
				auto expression = dynamic_cast<HXSLLiteralExpression*>(current);
				auto& token = expression->GetExpressionToken();

				HXSLSymbolDef* def = nullptr;
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

	void HXSLSymbolResolver::TypeCheckStatement(ASTNode*& node)
	{
		auto& type = node->GetType();
		switch (type)
		{
		case HXSLNodeType_DeclarationStatement:
		{
			auto statement = dynamic_cast<HXSLDeclarationStatement*>(node);
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

	HXSLTraversalBehavior HXSLSymbolResolver::TypeChecksExpression(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();
		if (!IsStatementType(type)) return HXSLTraversalBehavior_Keep;

		TypeCheckStatement(node);

		return HXSLTraversalBehavior_Keep;
	}

	static HXSLSymbolRef* GetResolvedTypeFromDecl(HXSLSymbolRef* ref)
	{
		auto decl = ref->GetDeclaration();
		if (auto typed = dynamic_cast<IHXSLHasSymbolRef*>(decl))
		{
			return typed->GetSymbolRef().get();
		}
		return nullptr;
	}

	int HXSLSymbolResolver::ResolveMemberInner(HXSLSymbolRef* type, IHXSLHasSymbolRef* getter) const
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
			if (metadata && metadata->Declaration->GetType() == HXSLNodeType_Primitive)
			{
				return swizzleManager->VerifySwizzle(metadata->Declaration->As<HXSLPrimitive>(), refInner);
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

	HXSLTraversalBehavior HXSLSymbolResolver::ResolveMember(HXSLMemberAccessExpression* memberAccessExpr, ASTNode*& next) const
	{
		auto getter = dynamic_cast<IHXSLHasSymbolRef*>(memberAccessExpr);
		auto chain = dynamic_cast<IHXSLChainExpression*>(memberAccessExpr);
		auto& refRoot = memberAccessExpr->GetSymbolRef();
		if (!ResolveSymbol(refRoot.get()))
		{
			return HXSLTraversalBehavior_Keep;
		}

		UseBeforeDeclarationCheck(refRoot.get(), memberAccessExpr);

		while (chain)
		{
			auto ref = getter->GetSymbolRef().get();

			HXSLSymbolRef* type = GetResolvedTypeFromDecl(ref);
			if (!type)
			{
				analyzer.LogError("Couldn't resolve type of member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return HXSLTraversalBehavior_Keep;
			}

			next = chain->chainNext().get();
			chain = dynamic_cast<IHXSLChainExpression*>(next);
			getter = dynamic_cast<IHXSLHasSymbolRef*>(next);
			if (getter == nullptr)
			{
				analyzer.LogError("Couldn't resolve member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return HXSLTraversalBehavior_Keep;
			}

			auto result = ResolveMemberInner(type, getter);
			if (result == -1)
			{
				analyzer.LogError("Couldn't resolve member '%s'", ref->GetSpan(), ref->GetSpan().toString().c_str());
				return HXSLTraversalBehavior_Keep;
			}
			else if (result == 1)
			{
				next = memberAccessExpr;
				return HXSLTraversalBehavior_Defer;
			}
		}

		return HXSLTraversalBehavior_Keep;
	}
}