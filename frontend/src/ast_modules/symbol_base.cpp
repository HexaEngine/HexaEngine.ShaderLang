#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "helpers.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	void SymbolDef::UpdateName()
	{
		std::string_view fqnView = *fullyQualifiedName.get();
		auto end = fqnView.find('(');
		if (end != std::string::npos)
		{
			fqnView = fqnView.substr(0, end);
		}

		auto pos = fqnView.find_last_of(QUALIFIER_SEP);
		if (pos != std::string::npos)
		{
			pos++;
			name = fqnView.substr(pos);
		}
		else
		{
			name = fqnView;
		}
	}

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
			fullyQualifiedName.reset();
			return;
		}

		if (fullyQualifiedName.get())
		{
			*fullyQualifiedName.get() = symbolHandle.GetFullyQualifiedName();
		}
		else
		{
			fullyQualifiedName = make_ast_ptr<std::string>(symbolHandle.GetFullyQualifiedName());
		}

		UpdateName();
	}

	const SymbolMetadata* SymbolDef::GetMetadata() const
	{
		auto& node = symbolHandle.GetNode();
		return node.Metadata.get();
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

	SymbolRef* SymbolRef::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* identifer, SymbolRefType type, bool isFullyQualified)
	{
		return context->Alloc<SymbolRef>(sizeof(SymbolRef), span, identifer, type, isFullyQualified);
	}

	void SymbolRef::TrimCastType()
	{
		std::string& result = *fullyQualifiedName.get();
		result.erase(std::remove(result.begin(), result.end(), '('), result.end());
		result.erase(std::remove(result.begin(), result.end(), ')'), result.end());
		UpdateName();
	}

	void SymbolRef::UpdateName()
	{
		name = *fullyQualifiedName.get();
		auto pos = name.find_last_of(QUALIFIER_SEP);
		if (pos != std::string::npos)
		{
			pos++;
			name = name.substr(pos);
		}
	}

	void SymbolRef::SetTable(const SymbolHandle& handle)
	{
		this->symbolHandle = handle;
		auto meta = GetMetadata();
		GetDeclaration()->AddRef(this);
		fullyQualifiedName = make_ast_ptr<std::string>(handle.GetFullyQualifiedName().c_str());
	}

	void SymbolRef::SetDeclaration(const SymbolDef* node)
	{
		SetTable(node->GetSymbolHandle());
	}

	const std::string& SymbolRef::GetFullyQualifiedName() const
	{
		return *fullyQualifiedName.get();
	}

	const SymbolMetadata* SymbolRef::GetMetadata() const
	{
		return symbolHandle.GetMetadata();
	}

	SymbolDef* SymbolRef::GetDeclaration() const
	{
		auto metadata = GetMetadata();
		if (metadata)
		{
			return metadata->declaration;
		}
		return nullptr;
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

	void SymbolRef::Write(Stream& stream) const
	{
		HXSL_ASSERT_DEPRECATION;
	}

	void SymbolRef::Read(Stream& stream)
	{
		HXSL_ASSERT_DEPRECATION;
	}

	ast_ptr<SymbolRef> SymbolRef::Clone(ASTContext* context) const
	{
		auto cloned = Create(context, span, identifer, type, isFullyQualified);
		cloned->identifer = identifer;
		cloned->span = span;
		cloned->type = type;
		cloned->symbolHandle = symbolHandle;
		cloned->arrayDims = arrayDims;
		cloned->isDeferred = isDeferred;
		cloned->notFound = notFound;
		return ast_ptr<SymbolRef>(cloned);
	}
}