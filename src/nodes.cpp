#include "nodes.hpp"
#include "symbol_table.hpp"
#include "string_pool.hpp"
#include "assembly_collection.hpp"

namespace HXSL
{
	inline void HXSLNode::AssignId()
	{
		auto compilation = GetCompilation();
		HXSL_ASSERT(compilation != nullptr, "Cannot assign ID compilation was null.");
		ID = compilation->GetNextID();
	}

	bool UsingDeclaration::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Target, AssemblyReferences);
		return AssemblyReferences.size() > 0;
	}

	void HXSLSymbolDef::SetAssembly(const Assembly* assembly, size_t tableIndex)
	{
		m_assembly = assembly;
		auto table = assembly->GetSymbolTable();
		TableIndex = tableIndex;
		FullyQualifiedName = std::make_unique<std::string>(table->GetFullyQualifiedName(tableIndex).c_str());
		if (isExtern)
		{
			std::string_view fqnView = *FullyQualifiedName.get();
			auto pos = fqnView.rfind(QUALIFIER_SEP);
			if (pos != std::string::npos)
			{
				pos++;
				Name = TextSpan(fqnView.data(), pos, fqnView.size() - pos, 0, 0);
			}
			else
			{
				Name = TextSpan(fqnView.data(), 0, fqnView.size(), 0, 0);
			}
		}
	}

	const SymbolMetadata* HXSLSymbolDef::GetMetadata() const
	{
		return GetTable()->GetNode(TableIndex).Metadata.get();
	}

	void HXSLSymbolRef::SetTable(const SymbolTable* table, size_t tableIndex)
	{
		Table = table;
		TableIndex = tableIndex;
		auto meta = GetMetadata();
		GetDeclaration()->AddRef(this);
		FullyQualifiedName = std::make_unique<std::string>(table->GetFullyQualifiedName(tableIndex).c_str());
	}

	const std::string& HXSLSymbolRef::GetFullyQualifiedName() const
	{
		return *FullyQualifiedName.get();
	}

	const SymbolMetadata* HXSLSymbolRef::GetMetadata() const
	{
		return Table->GetNode(TableIndex).Metadata.get();
	}

	HXSLSymbolDef* HXSLSymbolRef::GetDeclaration() const
	{
		if (Table == nullptr) return nullptr;
		return Table->GetNode(TableIndex).Metadata.get()->Declaration;
	}

	void HXSLSymbolRef::Write(HXSLStream& stream) const
	{
		stream.WriteUInt(Type);
		stream.WriteString(GetFullyQualifiedName());
	}

	void HXSLSymbolRef::Read(HXSLStream& stream)
	{
		Type = static_cast<HXSLSymbolRefType>(stream.ReadUInt());
		FullyQualifiedName = std::make_unique<std::string>(stream.ReadString().c_str());
		Span = TextSpan(*FullyQualifiedName.get());
	}

#define UNIQUE_PTR_CAST(ptr, type) \
std::move(std::unique_ptr<type>(static_cast<type*>(std::move(ptr).release())))

	void HXSLStruct::Write(HXSLStream& stream) const
	{
	}

	void HXSLStruct::Read(HXSLStream& stream, StringPool& container)
	{
	}

	void HXSLStruct::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->SymbolType)
			{
			case HXSLSymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], HXSLField));
				break;
			case HXSLSymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], HXSLFunction));
				break;
			case HXSLSymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], HXSLStruct));
				break;
			}
		}
	}

	void HXSLParameter::Write(HXSLStream& stream) const
	{
		stream.WriteUInt(Flags);
		stream.WriteString(Semantic);
		Symbol->Write(stream);
	}

	void HXSLParameter::Read(HXSLStream& stream, StringPool& container)
	{
		Flags = static_cast<HXSLParameterFlags>(stream.ReadUInt());
		Semantic = TextSpan(container.add(stream.ReadString()));
		Symbol = std::make_unique<HXSLSymbolRef>();
		Symbol->Read(stream);
	}

	void HXSLParameter::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
	}

	void HXSLField::Write(HXSLStream& stream) const
	{
		stream.WriteUInt(Flags);
		stream.WriteString(Semantic);
		Symbol->Write(stream);
	}

	void HXSLField::Read(HXSLStream& stream, StringPool& container)
	{
		Flags = static_cast<HXSLFieldFlags>(stream.ReadUInt());
		Semantic = TextSpan(container.add(stream.ReadString()));
		Symbol = std::make_unique<HXSLSymbolRef>();
		Symbol->Read(stream);
	}

	void HXSLField::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
	}

	void HXSLFunction::Write(HXSLStream& stream) const
	{
		stream.WriteUInt(Flags);
		stream.WriteString(Semantic);
		ReturnSymbol->Write(stream);
	}

	void HXSLFunction::Read(HXSLStream& stream, StringPool& container)
	{
		Flags = static_cast<HXSLFunctionFlags>(stream.ReadUInt());
		Semantic = TextSpan(container.add(stream.ReadString()));
		ReturnSymbol = std::make_unique<HXSLSymbolRef>();
		ReturnSymbol->Read(stream);
	}

	void HXSLFunction::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->SymbolType)
			{
			case HXSLNodeType_Parameter:
				AddParameter(UNIQUE_PTR_CAST(nodes[childIdx], HXSLParameter));
				break;
			}
		}
	}

	void HXSLDeclarationStatement::Write(HXSLStream& stream) const
	{
		//HXSL_ASSERT(false, "Cannot write declaration statements.");
	}

	void HXSLDeclarationStatement::Read(HXSLStream& stream, StringPool& container)
	{
		//HXSL_ASSERT(false, "Cannot read declaration statements.");
	}

	void HXSLDeclarationStatement::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
		//HXSL_ASSERT(false, "Cannot build declaration statements.");
	}

	void HXSLClass::Write(HXSLStream& stream) const
	{
	}

	void HXSLClass::Read(HXSLStream& stream, StringPool& container)
	{
	}

	void HXSLClass::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->SymbolType)
			{
			case HXSLSymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], HXSLField));
				break;
			case HXSLSymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], HXSLFunction));
				break;
			case HXSLSymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], HXSLStruct));
				break;
			}
		}
	}

	void HXSLNamespace::Write(HXSLStream& stream) const
	{
	}

	void HXSLNamespace::Read(HXSLStream& stream, StringPool& container)
	{
	}

	void HXSLNamespace::Build(SymbolTable& table, size_t index, HXSLCompilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->SymbolType)
			{
			case HXSLSymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], HXSLField));
				break;
			case HXSLSymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], HXSLFunction));
				break;
			case HXSLSymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], HXSLStruct));
				break;
			}
		}
	}

	void HXSLNamespace::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Name, References);
	}

	void HXSLContainer::AddFunction(std::unique_ptr<HXSLFunction> function)
	{
		function->SetParent(this);
		Functions.push_back(std::move(function));
	}

	void HXSLContainer::AddStruct(std::unique_ptr<HXSLStruct> _struct)
	{
		_struct->SetParent(this);
		Structs.push_back(std::move(_struct));
	}

	void HXSLContainer::AddClass(std::unique_ptr<HXSLClass> _class)
	{
		_class->SetParent(this);
		Classes.push_back(std::move(_class));
	}

	void HXSLContainer::AddField(std::unique_ptr<HXSLField> field)
	{
		field->SetParent(this);
		Fields.push_back(std::move(field));
	}
}