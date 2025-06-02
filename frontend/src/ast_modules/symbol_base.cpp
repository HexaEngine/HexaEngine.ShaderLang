#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "helpers.hpp"
#include "ast_context.hpp"

namespace HXSL
{
#pragma region SymbolDef

	void SymbolDef::SetAssembly(ASTContext* context, const Assembly* assembly, const SymbolHandle& handle)
	{
		this->assembly = assembly;
		this->symbolHandle = handle;
		UpdateFQN(context);
	}

	void SymbolDef::UpdateFQN(ASTContext* context)
	{
		if (symbolHandle.invalid())
		{
			fullyQualifiedName = nullptr;
			return;
		}

		fullyQualifiedName = context->GetIdentiferTable().Get(symbolHandle.GetFullyQualifiedName());
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

	ast_ptr<SymbolRef> SymbolDef::MakeSymbolRef(ASTContext* context) const
	{
		auto ref = ast_ptr<SymbolRef>(SymbolRef::Create(context, {}, context->GetIdentiferTable().Get(GetName()), ConvertSymbolTypeToSymbolRefType(GetSymbolType()), false));
		ref->SetTable(GetSymbolHandle());
		return ref;
	}

	const StringSpan& SymbolDef::ToString() const noexcept
	{
		return GetFullyQualifiedName();
	}

#pragma endregion

#pragma region SymbolRef

	SymbolRef* SymbolRef::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* identifer, SymbolRefType type, bool isFullyQualified)
	{
		return context->Alloc<SymbolRef>(sizeof(SymbolRef), span, identifer, type, isFullyQualified);
	}

	const StringSpan& SymbolRef::GetName() const noexcept
	{
		auto span = identifer->name.view();
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
		while (auto refPtr = SymbolRefHelper::TryGetSymbolRef(decl))
		{
			auto& ref = *refPtr;
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
		while (auto refPtr = SymbolRefHelper::TryGetSymbolRef(decl))
		{
			auto& ref = *refPtr;
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

	ast_ptr<SymbolRef> SymbolRef::Clone(ASTContext* context) const
	{
		auto cloned = Create(context, span, identifer, type, isFullyQualified);
		cloned->identifer = identifer;
		cloned->span = span;
		cloned->type = type;
		cloned->def = def;
		cloned->isDeferred = isDeferred;
		cloned->notFound = notFound;
		return ast_ptr<SymbolRef>(cloned);
	}

	const StringSpan& SymbolRef::ToString() const noexcept
	{
		return GetFullyQualifiedName();
	}

#pragma endregion

	SymbolRefArray* SymbolRefArray::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ArrayRef<size_t> arrayDims)
	{
		auto ptr = context->Alloc<SymbolRefArray>(TotalSizeToAlloc(arrayDims.size()), span, name);
		ptr->numArrayDims = static_cast<uint32_t>(arrayDims.size());
		std::uninitialized_move(arrayDims.begin(), arrayDims.end(), ptr->GetArrayDims().data());
		return ptr;
	}

	SymbolRefArray* SymbolRefArray::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, uint32_t numArrayDims)
	{
		auto ptr = context->Alloc<SymbolRefArray>(TotalSizeToAlloc(numArrayDims), span, name);
		ptr->numArrayDims = numArrayDims;
		ptr->GetArrayDims().init();
		return ptr;
	}
}