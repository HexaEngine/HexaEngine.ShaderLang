#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "helpers.hpp"
#include "ast_context.hpp"

namespace HXSL
{
#pragma region SymbolDef

	void SymbolDef::SetAssembly(const Assembly* assembly, const SymbolHandle& handle)
	{
		this->assembly = assembly;
		this->symbolHandle = handle;
		UpdateFQN();
	}

	void SymbolDef::UpdateFQN()
	{
		if (symbolHandle.invalid())
		{
			fullyQualifiedName = nullptr;
			return;
		}

		auto* context = ASTContext::GetCurrentContext();
		fullyQualifiedName = context->GetIdentifierTable().Get(symbolHandle.GetFullyQualifiedName());
	}

	const StringSpan& SymbolDef::GetName() const
	{
		return identifer->name.view();
	}

	const StringSpan& SymbolDef::GetFullyQualifiedName() const noexcept
	{
		return fullyQualifiedName->name.view();
	}

	const SymbolMetadata* SymbolDef::GetMetadata() const
	{
		auto& node = symbolHandle.GetNode();
		return node.Metadata.get();
	}

	bool SymbolDef::IsConstant() const
	{
		switch (type)
		{
		case NodeType_Field: return (cast<Field>(this)->GetStorageClass() & StorageClass_Const) != 0;
		case NodeType_DeclarationStatement: return (cast<DeclarationStatement>(this)->GetStorageClass() & StorageClass_Const) != 0;
		}
		return false;
	}

	SymbolType SymbolDef::GetSymbolType() const
	{
		switch (type)
		{
		case NodeType_Namespace: return SymbolType_Namespace;
		case NodeType_Enum: return SymbolType_Enum;
		case NodeType_Primitive: return SymbolType_Struct;
		case NodeType_Struct: return SymbolType_Struct;
		case NodeType_Class: return SymbolType_Class;
		case NodeType_Array: return SymbolType_Array;
		case NodeType_Pointer: return SymbolType_Pointer;
		case NodeType_Field: return SymbolType_Field;
		case NodeType_IntrinsicFunction: return SymbolType_IntrinsicFunction;
		case NodeType_FunctionOverload: return SymbolType_Function;
		case NodeType_OperatorOverload: return SymbolType_Operator;
		case NodeType_ConstructorOverload: return SymbolType_Constructor;
		case NodeType_ThisDef: return SymbolType_Variable;
		case NodeType_SwizzleDefinition: return SymbolType_Field;
		case NodeType_DeclarationStatement: return SymbolType_Variable;
		case NodeType_Parameter: return SymbolType_Variable;
		default:
			break;
		}
		HXSL_ASSERT(false, "Unhandled SymbolDef type.");
		return SymbolType_Unknown;
	}

	SymbolRef* SymbolDef::MakeSymbolRef() const
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ref = SymbolRef::Create({}, context->GetIdentifierTable().Get(GetName()), ConvertSymbolTypeToSymbolRefType(GetSymbolType()), false);
		ref->SetTable(GetSymbolHandle());
		return ref;
	}

	const StringSpan& SymbolDef::ToString() const noexcept
	{
		return GetFullyQualifiedName();
	}

#pragma endregion

#pragma region SymbolRef

	SymbolRef* SymbolRef::Create(const TextSpan& span, IdentifierInfo* identifier, SymbolRefType type, bool isFullyQualified)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<SymbolRef>(sizeof(SymbolRef), span, identifier, type, isFullyQualified);
	}

	const StringSpan& SymbolRef::GetName() const noexcept
	{
		auto span = identifier->name.view();
		if (isFullyQualified)
		{
			auto pos = span.find_last_of(QUALIFIER_SEP);
			if (pos != std::string::npos)
			{
				pos++;
				span = span.substr(pos);
			}
		}
		return span;
	}

	void SymbolRef::SetTable(const SymbolHandle& handle)
	{
		def = handle.GetMetadata()->declaration;
	}

	void SymbolRef::SetDeclaration(const SymbolDef* node)
	{
		SetTable(node->GetSymbolHandle());
	}

	void SymbolRef::TrimCastType()
	{
		StringSpan result = identifier->name;
		result = result.trim_start('(');
		result = result.trim_end(')');
		identifier = ASTContext::GetCurrentContext()->GetIdentifier(result);
	}

	const SymbolHandle& SymbolRef::GetSymbolHandle() const
	{
		return def->GetSymbolHandle();
	}

	const StringSpan& SymbolRef::GetFullyQualifiedName() const
	{
		return def->GetFullyQualifiedName();
	}

	SymbolDef* SymbolRef::GetDeclaration() const
	{
		return def;
	}

	SymbolDef* SymbolRef::GetBaseDeclaration() const
	{
		std::unordered_set<const SymbolDef*> visited;
		auto decl = GetDeclaration();
		while (auto ref = SymbolRefHelper::TryGetSymbolRef(decl))
		{
			if (!visited.insert(decl).second)
			{
				return nullptr;
			}
			decl = ref->GetDeclaration();
		}
		return decl;
	}

	SymbolDef* SymbolRef::GetAncestorDeclaration(SymbolType type) const
	{
		std::unordered_set<const SymbolDef*> visited;
		auto decl = GetDeclaration();
		while (auto ref = SymbolRefHelper::TryGetSymbolRef(decl))
		{
			if (!visited.insert(decl).second)
			{
				return nullptr;
			}
			decl = ref->GetDeclaration();
			if (decl->GetSymbolType() == type)
			{
				return decl;
			}
		}
		return decl;
	}

	SymbolRef* SymbolRef::Clone() const
	{
		auto cloned = Create(span, identifier, type, isFullyQualified);
		cloned->identifier = identifier;
		cloned->span = span;
		cloned->type = type;
		cloned->def = def;
		cloned->isDeferred = isDeferred;
		cloned->notFound = notFound;
		return cloned;
	}

	const StringSpan& SymbolRef::ToString() const noexcept
	{
		return GetFullyQualifiedName();
	}

#pragma endregion

	SymbolRefArray* SymbolRefArray::Create(const TextSpan& span, IdentifierInfo* name, const Span<size_t>& arrayDims)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<SymbolRefArray>(TotalSizeToAlloc(arrayDims.size()), span, name);
		ptr->storage.InitializeMove(ptr, arrayDims);
		return ptr;
	}

	SymbolRefArray* SymbolRefArray::Create(const TextSpan& span, IdentifierInfo* name, uint32_t numArrayDims)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<SymbolRefArray>(TotalSizeToAlloc(numArrayDims), span, name);
		ptr->storage.SetCounts(numArrayDims);
		return ptr;
	}
}