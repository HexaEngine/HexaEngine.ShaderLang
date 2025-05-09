#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "symbols/symbol_table.hpp"

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
		fullyQualifiedName = std::make_unique<std::string>(symbolHandle.GetFullyQualifiedName());
		UpdateName();
	}

	const SymbolMetadata* SymbolDef::GetMetadata() const
	{
		auto& node = symbolHandle.GetNode();
		return node.Metadata.get();
	}

	std::unique_ptr<SymbolRef> SymbolDef::MakeSymbolRef() const
	{
		auto ref = std::make_unique<SymbolRef>(std::string(GetName()), ConvertSymbolTypeToSymbolRefType(GetSymbolType()), false);
		ref->SetTable(GetSymbolHandle());
		return ref;
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
		fullyQualifiedName = std::make_unique<std::string>(handle.GetFullyQualifiedName().c_str());
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
		while (auto getter = dynamic_cast<IHasSymbolRef*>(decl))
		{
			if (!visited.insert(decl).second)
			{
				return nullptr;
			}
			decl = getter->GetSymbolRef()->GetDeclaration();
		}
		return decl;
	}

	SymbolDef* SymbolRef::GetAncestorDeclaration(SymbolType type) const
	{
		std::unordered_set<const SymbolDef*> visited;
		auto decl = GetDeclaration();
		while (auto getter = dynamic_cast<IHasSymbolRef*>(decl))
		{
			if (!visited.insert(decl).second)
			{
				return nullptr;
			}
			decl = getter->GetSymbolRef()->GetDeclaration();
			if (decl->GetSymbolType() == type)
			{
				return decl;
			}
		}
		return decl;
	}

	void SymbolRef::Write(Stream& stream) const
	{
		stream.WriteUInt(type);
		stream.WriteString(GetFullyQualifiedName());
	}

	void SymbolRef::Read(Stream& stream)
	{
		type = static_cast<SymbolRefType>(stream.ReadUInt());
		fullyQualifiedName = std::make_unique<std::string>(stream.ReadString().c_str());
		UpdateName();
	}
}