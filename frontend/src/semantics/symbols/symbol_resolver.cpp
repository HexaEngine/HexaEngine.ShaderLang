#include "symbol_resolver.hpp"

namespace HXSL
{
	bool SymbolResolver::SymbolTypeSanityCheck(const SymbolMetadata* metadata, SymbolRef* ref, bool silent) const
	{
		if (metadata == nullptr)
		{
			HXSL_ASSERT(false, "SymbolMetadata was null, this should never happen.");
			return false;
		}

		auto refType = ref->GetType();
		auto defType = metadata->symbolType;
		auto& name = ref->GetName();
		auto& span = ref->GetSpan();

		switch (refType)
		{
		case SymbolRefType_Unknown:
			HXSL_ASSERT(false, "SymbolRef was type of unknown, this should never happen.");
			return false;

		case SymbolRefType_Namespace:
			if (defType != SymbolType_Namespace)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_NAMESPACE_SYMBOL, span, name, ToString(defType));
				}

				return false;
			}
			break;

		case SymbolRefType_FunctionOverload:
			if (defType != SymbolType_Function)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_FUNC_SYMBOL, span, name, ToString(defType));
				}

				return false;
			}
			break;

		case SymbolRefType_OperatorOverload:
			if (defType != SymbolType_Operator)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_OP_SYMBOL, span, name, ToString(defType));
				}

				return false;
			}
			break;

		case SymbolRefType_Constructor:
			if (defType != SymbolType_Constructor)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_CTOR_SYMBOL, span, name, ToString(defType));
				}

				return false;
			}
			break;

		case SymbolRefType_FunctionOrConstructor:
			if (defType != SymbolType_Function && defType != SymbolType_Constructor)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_CTOR_OR_FUNC_SYMBOL, span, name, ToString(defType));
				}

				return false;
			}
			break;

		case SymbolRefType_Struct:
			if (defType != SymbolType_Struct && defType != SymbolType_Primitive)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_STRUCT_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_Enum:
			if (defType != SymbolType_Enum)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_ENUM_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_Identifier:
			if (defType != SymbolType_Field && defType != SymbolType_Parameter && defType != SymbolType_Variable && defType != SymbolType_Struct && defType != SymbolType_Primitive && defType != SymbolType_Class && defType != SymbolType_Enum)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_IDENTIFIER_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_Attribute:
			if (defType != SymbolType_Attribute)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_ATTRIBUTE_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_Member:
			if (defType != SymbolType_Field)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_MEMBER_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_Type:
			if (defType != SymbolType_Struct && defType != SymbolType_Primitive && defType != SymbolType_Class && defType != SymbolType_Enum)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_TYPE_SYMBOL, span, name, ToString(defType));
				}
				return false;
			}
			break;

		case SymbolRefType_ArrayType:
			if (defType != SymbolType_Array)
			{
				if (!silent)
				{
					analyzer.Log(EXPECTED_ARRAY_SYMBOL, span, name, ToString(defType));
				}
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

	bool SymbolResolver::SymbolVisibilityChecks(const SymbolMetadata* metadata, SymbolRef* ref, ResolverScopeContext& context) const
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

	bool SymbolResolver::TryResolveThis(SymbolHandle& outHandle, SymbolDef*& outDefinition) const
	{
		auto local = targetAssembly->GetSymbolTable();
		for (auto it = stack.rbegin(); it != stack.rend(); ++it)
		{
			auto& scope = *it;
			auto type = scope.Parent->GetType();
			if (type == NodeType_Class || type == NodeType_Struct)
			{
				outDefinition = cast<SymbolDef>(scope.Parent);
				outHandle = outDefinition->GetSymbolHandle();
				return true;
			}
		}
		return false;
	}

	bool SymbolResolver::TryResolve(const SymbolTable* table, const StringSpan& name, const SymbolHandle& lookup, SymbolHandle& outHandle, SymbolDef*& outDefinition) const
	{
		auto handle = lookup.FindFullPath(name, table);
		if (handle.valid())
		{
			outHandle = handle;
			outDefinition = handle.GetMetadata()->declaration;
			return true;
		}
		outDefinition = nullptr;
		return false;
	}

	bool SymbolResolver::TryResolveFromRoot(const SymbolTable* table, const StringSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const
	{
		auto handle = table->FindNodeIndexFullPath(name);
		if (handle.valid())
		{
			outHandle = handle;
			outDefinition = handle.GetMetadata()->declaration;
			return true;
		}
		outDefinition = nullptr;
		return false;
	}

	bool SymbolResolver::TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const StringSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const
	{
		for (auto& ref : references)
		{
			auto table = ref.TargetAssembly->GetSymbolTable();
			auto& lookup = ref.LookupHandle;
			if (TryResolve(table, name, ref.LookupHandle, outHandle, outDefinition))
			{
				return true;
			}
		}

		return false;
	}

	SymbolDef* SymbolResolver::ResolveSymbol(const TextSpan& span, const StringSpan& name, bool isFullyQualified, SymbolHandle& outHandle, bool silent) const
	{
		SymbolDef* def;
		if (isFullyQualified)
		{
			if (TryResolveFromRoot(primitives.GetSymbolTable(), name, outHandle, def))
			{
				return def;
			}

			auto currentNamespace = current.CurrentNamespace;
			if (TryResolveFromRoot(currentNamespace->GetTable(), name, outHandle, def))
			{
				return def;
			}

			for (auto& ref : currentNamespace->GetAssemblyReferences())
			{
				auto table = ref.TargetAssembly->GetSymbolTable();
				if (TryResolveFromRoot(table, name, outHandle, def))
				{
					return def;
				}
			}

			for (auto& assembly : references.GetAssemblies())
			{
				auto table = assembly->GetSymbolTable();
				if (TryResolveFromRoot(table, name, outHandle, def))
				{
					return def;
				}
			}

			if (!silent)
			{
				analyzer.Log(SYMBOL_NOT_FOUND, span, name.str());
			}
			return nullptr;
		}

		if (name == "this")
		{
			if (TryResolveThis(outHandle, def))
			{
				return def;
			}
			if (!silent)
			{
				analyzer.Log(SYMBOL_NOT_FOUND, span, name.str());
			}
			return nullptr;
		}

		if (TryResolveFromRoot(primitives.GetSymbolTable(), name, outHandle, def))
		{
			return def;
		}

		auto local = targetAssembly->GetSymbolTable();
		if (TryResolve(local, name, current.SymbolHandle, outHandle, def))
		{
			return def;
		}

		for (auto it = stack.rbegin(); it != stack.rend(); ++it)
		{
			auto& scope = *it;
			if (TryResolve(local, name, scope.SymbolHandle, outHandle, def))
			{
				return def;
			}
		}

		auto currentNamespace = current.CurrentNamespace;
		if (TryResolveInAssemblies(currentNamespace->GetAssemblyReferences(), name, outHandle, def))
		{
			return def;
		}

		for (auto& us : currentNamespace->GetUsings())
		{
			if (us.IsAlias) continue;
			if (TryResolveInAssemblies(us.AssemblyReferences, name, outHandle, def))
			{
				return def;
			}
		}

		for (auto& us : compilation->GetUsings())
		{
			if (us.IsAlias) continue;
			if (TryResolveInAssemblies(us.AssemblyReferences, name, outHandle, def))
			{
				return def;
			}
		}

		if (!silent)
		{
			analyzer.Log(SYMBOL_NOT_FOUND, span, name.str());
		}

		return nullptr;
	}

	SymbolDef* SymbolResolver::ResolvePrimitiveSymbol(const std::string& str) const
	{
		SymbolHandle handle;
		SymbolDef* def;
		if (TryResolveFromRoot(primitives.GetSymbolTable(), str, handle, def))
		{
			return def;
		}

		HXSL_ASSERT(false, "Couldn't find primitive.");
		return nullptr;
	}

	bool SymbolResolver::ResolveSymbol(SymbolRef* ref, std::optional<StringSpan> name, bool silent) const
	{
		if (ref->IsResolved()) return true;
		bool isFQN;
		StringSpan span;
		if (name.has_value())
		{
			isFQN = false;
			span = name.value();
		}
		else
		{
			isFQN = ref->HasFullyQualifiedName();
			span = isFQN ? ref->GetFullyQualifiedName() : ref->GetName();
		}

		SymbolHandle handle;
		auto def = ResolveSymbol(ref->GetSpan(), span, isFQN, handle, silent);
		if (!def)
		{
			ref->SetNotFound(true);
			return false;
		}

		if (ref->IsArray())
		{
			if (!arrayManager->TryGetOrCreateArrayType(ref, def, handle, def))
			{
				ref->SetNotFound(true);
				analyzer.Log(INVALID_ARRAY_TYPE, ref->GetSpan(), def->ToString());
				return false;
			}
		}

		auto metadata = handle.GetMetadata();
		if (!SymbolTypeSanityCheck(metadata, ref, silent))
		{
			ref->SetNotFound(true);
			return false;
		}
		ref->SetTable(handle);

		return true;
	}

	bool SymbolResolver::ResolveSymbol(SymbolRef* ref, std::optional<StringSpan> name, const SymbolTable* table, const SymbolHandle& lookup, bool silent) const
	{
		auto& span = ref->GetName();
		auto actualName = name.value_or(span);

		SymbolHandle handle;
		SymbolDef* def;
		if (!TryResolve(table, actualName, lookup, handle, def))
		{
			ref->SetNotFound(true);
			if (!silent)
			{
				analyzer.Log(SYMBOL_NOT_FOUND, ref->GetSpan(), actualName.str());
			}
			return false;
		}

		auto metadata = handle.GetMetadata();
		if (!SymbolTypeSanityCheck(metadata, ref, silent))
		{
			ref->SetNotFound(true);
			return false;
		}
		ref->SetTable(handle);

		return true;
	}

	bool SymbolResolver::TryResolveConstructor(FunctionCallExpression* funcCallExpr, SymbolDef*& outDefinition, bool* success, bool silent) const
	{
		if (success) *success = false;
		outDefinition = nullptr;

		SymbolRef* ref = funcCallExpr->GetSymbolRef().get();

		SymbolHandle handle;
		SymbolDef* typeDef = ResolveSymbol({}, ref->GetName(), ref->HasFullyQualifiedName(), handle, true);

		if (handle.invalid() || typeDef == nullptr || !IsDataType(typeDef->GetType()))
		{
			return false;
		}

		auto signature = funcCallExpr->BuildConstructorOverloadSignature();
		auto ctorHandle = handle.FindPart(signature);
		if (ctorHandle.valid())
		{
			auto ctorMetadata = ctorHandle.GetMetadata();
			if (SymbolTypeSanityCheck(ctorMetadata, ref, silent))
			{
				ref->SetTable(ctorHandle);
				outDefinition = ctorMetadata->declaration;
				if (success) *success = true;
				return true;
			}
		}

		if (!silent)
		{
			analyzer.Log(CTOR_OVERLOAD_NOT_FOUND, ref->GetSpan(), signature, typeDef->GetName());
		}
		return true;
	}

	bool SymbolResolver::ResolveFunction(FunctionCallExpression* funcCallExpr, SymbolDef*& outDefinition, bool silent) const
	{
		SymbolRef* ref = funcCallExpr->GetSymbolRef().get();

		auto signature = funcCallExpr->BuildOverloadSignature();

		bool success = false;
		if (auto expr = funcCallExpr->FindAncestor<MemberAccessExpression>(NodeType_MemberAccessExpression, 1))
		{
			auto memberRef = expr->GetSymbolRef()->GetBaseDeclaration();
			if (memberRef)
			{
				success = ResolveSymbol(ref, signature, memberRef->GetTable(), memberRef->GetSymbolHandle(), true);
			}
		}
		else
		{
			success = ResolveSymbol(ref, signature, true);
		}

		if (!success && !silent)
		{
			analyzer.Log(FUNC_OVERLOAD_NOT_FOUND, funcCallExpr->GetSpan(), signature);
		}

		if (success)
		{
			outDefinition = ref->GetDeclaration();
		}

		return success;
	}

	bool SymbolResolver::ResolveCallable(FunctionCallExpression* funcCallExpr, SymbolDef*& outDefinition, bool silent) const
	{
		bool success = false;
		if (TryResolveConstructor(funcCallExpr, outDefinition, &success, silent))
		{
			return success;
		}

		if (ResolveFunction(funcCallExpr, outDefinition, silent))
		{
			return true;
		}

		return false;
	}

	void SymbolResolver::PushScope(ASTNode* parent, const StringSpan& span, bool external)
	{
		stack.push(current);
		current.Parent = parent;
		if (external)
		{
			bool found = false;
			for (auto& assembly : references.GetAssemblies())
			{
				auto table = assembly->GetSymbolTable();
				auto index = current.SymbolHandle.FindFullPath(span, table);
				if (index.valid())
				{
					current.SymbolHandle = index;
					found = true;
					break;
				}
			}

			HXSL_ASSERT(found, "Couldn't resolve external node.");
		}
		else
		{
			auto local = targetAssembly->GetSymbolTable();
			current.SymbolHandle = current.SymbolHandle.FindFullPath(span, local);
		}
		HXSL_ASSERT(current.SymbolHandle.valid(), "Invalid index");
	}

	void SymbolResolver::PopScope()
	{
		current = stack.top();
		stack.pop();
	}

	void SymbolResolver::VisitClose(ASTNode* node, size_t depth)
	{
		if (current.Parent != node) return;
		PopScope();
	}

	TraversalBehavior SymbolResolver::VisitExternal(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case NodeType_Namespace:
		{
			auto currentNamespace = current.CurrentNamespace = cast<Namespace>(node);
			PushScope(node, currentNamespace->GetName(), true);
		}
		break;
		case NodeType_Struct:
		{
			PushScope(node, cast<Struct>(node)->GetName(), true);
		}
		break;
		case NodeType_Field:
		{
			auto field = cast<Field>(node);
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_FunctionOverload:
		{
			auto function = cast<FunctionOverload>(node);
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, signature, true);
		}
		break;
		case NodeType_ConstructorOverload:
		{
			auto function = cast<ConstructorOverload>(node);
			auto& ref = function->GetTargetTypeSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, signature, true);
		}
		break;
		case NodeType_OperatorOverload:
		{
			auto function = cast<OperatorOverload>(node);
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildOverloadSignature();
			PushScope(node, signature, true);
		}
		break;
		case NodeType_Parameter:
		{
			auto parameter = cast<Parameter>(node);
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
			if (auto declStmt = dyn_cast<DeclarationStatement>(decl))
			{
				if (parent->GetSpan().start < declStmt->GetSpan().start)
				{
					analyzer.Log(USE_BEFORE_DECL, parent->GetSpan(), ref->ToString());
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
			auto currentNamespace = current.CurrentNamespace = cast<Namespace>(node);
			PushScope(node, currentNamespace->GetName(), false);
		}
		break;
		case NodeType_Struct:
			PushScope(node, cast<Struct>(node)->GetName());
			break;
		case NodeType_Field:
		{
			auto field = cast<Field>(node);
			auto& ref = field->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_FunctionOverload:
		{
			auto function = cast<FunctionOverload>(node);
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildTemporaryOverloadSignature();
			PushScope(node, signature);
		}
		break;
		case NodeType_ConstructorOverload:
		{
			auto function = cast<ConstructorOverload>(node);
			auto& ref = function->GetTargetTypeSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildTemporaryOverloadSignature();
			PushScope(node, signature);
		}
		break;
		case NodeType_OperatorOverload:
		{
			auto function = cast<OperatorOverload>(node);
			auto& ref = function->GetReturnSymbolRef();
			ResolveSymbol(ref.get());
			auto signature = function->BuildTemporaryOverloadSignature();
			PushScope(node, signature);
		}
		break;
		case NodeType_Parameter:
		{
			auto parameter = cast<Parameter>(node);
			auto& ref = parameter->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_BlockStatement:
		{
			std::string temp = MakeScopeId(current.ScopeCounter++);
			PushScope(node, temp);
		}
		break;
		case NodeType_MemberReferenceExpression:
		{
			auto symbolRefExpression = cast<MemberReferenceExpression>(node);
			auto& ref = symbolRefExpression->GetSymbolRef();
			ResolveSymbol(ref.get());
			UseBeforeDeclarationCheck(ref.get(), node);
		}
		break;
		case NodeType_AttributeDeclaration:
		{
			auto attr = cast<AttributeDeclaration>(node);
			auto& ref = attr->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_DeclarationStatement:
		{
			auto declStatement = cast<DeclarationStatement>(node);
			auto& ref = declStatement->GetSymbolRef();
			ResolveSymbol(ref.get());
		}
		break;
		case NodeType_IndexerAccessExpression:
		{
			auto idxAccessExpression = cast<IndexerAccessExpression>(node);
			if (idxAccessExpression->GetParent()->GetType() != NodeType_IndexerAccessExpression)
			{
				auto& ref = idxAccessExpression->GetSymbolRef();
				ResolveSymbol(ref.get());
			}
		}
		break;
		case NodeType_MemberAccessExpression:
		{
			if (deferred)
			{
				current = context.current;
				stack = context.stack;
			}
			auto chainExpr = cast<ChainExpression>(node);
			auto next = node;
			auto result = ResolveMember(chainExpr, next);
			if (!deferred && result == TraversalBehavior_Defer)
			{
				context = ResolverDeferralContext(current, stack);
			}
			node = next;
			return result;
		}
		break;
		case NodeType_CastExpression:
		{
			auto castExpr = cast<CastExpression>(node);
			auto& ref = castExpr->GetTypeSymbol();
			ResolveSymbol(ref.get());
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

	ResolveMemberResult SymbolResolver::ResolveMemberInner(ChainExpression* expr, SymbolRef* type) const
	{
		auto refInner = expr->GetSymbolRef().get();
		auto& handle = type->GetSymbolHandle();

		auto exprType = expr->GetType();
		if (exprType == NodeType_FunctionCallExpression || exprType == NodeType_IndexerAccessExpression)
		{
			return ResolveMemberResult::Skip;
		}

		if (type->IsNotFound())
		{
			return ResolveMemberResult::Failure;
		}

		if (handle.invalid())
		{
			refInner->SetDeferred(true);
			return ResolveMemberResult::Defer;
		}

		refInner->SetDeferred(false);

		auto indexNext = handle.FindPart(refInner->GetName());
		if (indexNext.invalid())
		{
			auto metadata = type->GetMetadata();
			if (metadata && metadata->declaration->GetType() == NodeType_Primitive)
			{
				if (swizzleManager->VerifySwizzle(cast<Primitive>(metadata->declaration), refInner))
				{
					return ResolveMemberResult::Success;
				}
			}
			return ResolveMemberResult::Failure;
		}
		auto metaInner = indexNext.GetMetadata();
		if (!SymbolTypeSanityCheck(metaInner, refInner))
		{
			return ResolveMemberResult::Failure;
		}
		refInner->SetTable(indexNext);

		return ResolveMemberResult::Success;
	}

	TraversalBehavior SymbolResolver::ResolveMember(ChainExpression* chainExprRoot, ASTNode*& next, bool skipInitialResolve) const
	{
		ChainExpression* chain = chainExprRoot;

		if (!skipInitialResolve)
		{
			auto& refRoot = chainExprRoot->GetSymbolRef();
			if (!ResolveSymbol(refRoot.get()))
			{
				return TraversalBehavior_Keep;
			}

			UseBeforeDeclarationCheck(refRoot.get(), chainExprRoot);
		}

		while (chain)
		{
			auto ref = chain->GetSymbolRef().get();

			SymbolRef* type = GetResolvedTypeFromDecl(ref);
			if (!type)
			{
				analyzer.Log(CANNOT_RESOLVE_MEMBER_TYPE, ref->GetSpan(), ref->ToString());
				return TraversalBehavior_Keep;
			}

			chain = chain->GetNextExpression().get();

			if (chain == nullptr)
			{
				return TraversalBehavior_Keep; // terminal node here.
			}

			next = chain;

			auto result = ResolveMemberInner(chain, type);
			if (result == ResolveMemberResult::Failure)
			{
				auto refInner = chain->GetSymbolRef().get();
				analyzer.Log(MEMBER_NOT_FOUND_IN, refInner->GetSpan(), refInner->ToString(), ref->ToString());
				return TraversalBehavior_Keep;
			}
			else if (result == ResolveMemberResult::Defer)
			{
				next = chainExprRoot;
				return TraversalBehavior_Defer;
			}
			else if (result == ResolveMemberResult::Skip)
			{
				return TraversalBehavior_Keep;
			}
		}

		return TraversalBehavior_Keep;
	}
}