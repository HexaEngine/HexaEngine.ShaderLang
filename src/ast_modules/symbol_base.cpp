#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "symbols/symbol_table.hpp"

namespace HXSL
{
	void SymbolDef::SetAssembly(const Assembly* assembly, const SymbolHandle& handle)
	{
		this->assembly = assembly;
		this->symbolHandle = handle;
		fullyQualifiedName = std::make_unique<std::string>(symbolHandle.GetFullyQualifiedName());
		if (isExtern)
		{
			TextSpan fqnView = *fullyQualifiedName.get();
			auto end = fqnView.indexOf('(');
			if (end != std::string::npos)
			{
				fqnView = fqnView.slice(0, end);
			}

			auto pos = fqnView.lastIndexOf(QUALIFIER_SEP);
			if (pos != std::string::npos)
			{
				pos++;
				name = fqnView.slice(pos);
			}
			else
			{
				name = fqnView;
			}
		}
	}

	const SymbolMetadata* SymbolDef::GetMetadata() const
	{
		auto& node = symbolHandle.GetNode();
		return node.Metadata.get();
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
		return symbolHandle.GetNode().Metadata.get();
	}

	SymbolDef* SymbolRef::GetDeclaration() const
	{
		return symbolHandle.GetNode().Metadata.get()->declaration;
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

	void SymbolRef::Write(Stream& stream) const
	{
		stream.WriteUInt(type);
		stream.WriteString(GetFullyQualifiedName());
	}

	void SymbolRef::Read(Stream& stream)
	{
		type = static_cast<SymbolRefType>(stream.ReadUInt());
		fullyQualifiedName = std::make_unique<std::string>(stream.ReadString().c_str());
		span = TextSpan(*fullyQualifiedName.get());
		auto pos = span.lastIndexOf(QUALIFIER_SEP);
		if (pos != std::string::npos)
		{
			pos++;
			span = span.slice(pos);
		}
	}
}