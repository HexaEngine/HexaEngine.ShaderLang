#include "symbol_resolver.hpp"

namespace HXSL
{
	bool SymbolResolver::SymbolTypeSanityCheck(SymbolMetadata* metadata, SymbolRef* ref) const
	{
		auto refType = ref->GetType();
		auto defType = metadata->symbolType;
		auto& span = ref->GetName();

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

		case SymbolRefType_OperatorOverload:
			if (defType != SymbolType_Operator)
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
			HXSL_ASSERT(false, "Any symbol reference type.");
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

	bool SymbolResolver::ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name) const
	{
		auto& span = ref->GetName();
		auto& actualName = name.value_or(span);

		const SymbolTable* table;
		size_t index;
		auto def = ResolveSymbol(actualName, table, index);
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

	bool SymbolResolver::ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name, const SymbolTable* table, size_t nodeIndex) const
	{
		auto& span = ref->GetName();
		auto& actualName = name.value_or(span);

		SymbolDef* def;
		if (!TryResolve(table, actualName, nodeIndex, table, nodeIndex, def))
		{
			analyzer.LogError("Symbol not found '%s'", span, actualName.toString().c_str());
			return false;
		}

		auto metadata = table->GetNode(nodeIndex).Metadata.get();
		if (!SymbolTypeSanityCheck(metadata, ref))
		{
			return false;
		}
		ref->SetTable(table, nodeIndex);

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
		case NodeType_FunctionOverload:
		{
			auto function = node->As<FunctionOverload>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, TextSpan(signature), true);
		}
		break;
		case NodeType_OperatorOverload:
		{
			auto function = node->As<OperatorOverload>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, TextSpan(signature), true);
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
				if (usageID < declID) // this works because IDs are incremental.
				{
					auto refStr = ref->GetName().toString();
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
		case NodeType_FunctionOverload:
		{
			auto function = node->As<FunctionOverload>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, TextSpan(signature));
		}
		break;
		case NodeType_OperatorOverload:
		{
			auto function = node->As<OperatorOverload>();
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, TextSpan(signature));
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

	static SymbolRef* GetResolvedTypeFromDecl(SymbolRef* ref)
	{
		auto decl = ref->GetDeclaration();
		if (auto typed = dynamic_cast<IHasSymbolRef*>(decl))
		{
			return typed->GetSymbolRef().get();
		}
		return nullptr;
	}

	int SymbolResolver::ResolveMemberInner(SymbolRef* type, SymbolRef* refInner) const
	{
		auto table = type->GetTable();
		auto index = type->GetTableIndex();

		if (table == nullptr)
		{
			refInner->SetDeferred(true);
			return 1;
		}

		refInner->SetDeferred(false);

		auto& refType = refInner->GetType();
		if (refType == SymbolRefType_Function || refType == SymbolRefType_Constructor || refType == SymbolRefType_FunctionOrConstructor)
		{
			return 0; // do not resolve function calls.
		}

		auto indexNext = table->FindNodeIndexPart(refInner->GetName(), index);
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
			return TraversalBehavior_Skip;
		}

		UseBeforeDeclarationCheck(refRoot.get(), memberAccessExpr);

		while (chain)
		{
			auto ref = getter->GetSymbolRef().get();

			SymbolRef* type = GetResolvedTypeFromDecl(ref);
			if (!type)
			{
				analyzer.LogError("Couldn't resolve type of member '%s'", ref->GetName(), ref->GetName().toString().c_str());
				return TraversalBehavior_Skip;
			}

			next = chain->chainNext().get();
			chain = dynamic_cast<IChainExpression*>(next);
			getter = dynamic_cast<IHasSymbolRef*>(next);
			if (getter == nullptr)
			{
				analyzer.LogError("Couldn't resolve member '%s'", ref->GetName(), ref->GetName().toString().c_str());
				return TraversalBehavior_Skip;
			}

			auto refInner = getter->GetSymbolRef().get();
			auto result = ResolveMemberInner(type, refInner);
			if (result == -1)
			{
				analyzer.LogError("Couldn't resolve member '%s'", refInner->GetName(), refInner->GetName().toString().c_str());
				return TraversalBehavior_Skip;
			}
			else if (result == 1)
			{
				next = memberAccessExpr;
				return TraversalBehavior_Defer;
			}
		}

		return TraversalBehavior_Skip;
	}
}