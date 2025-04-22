#include "ast.hpp"
#include "symbols/symbol_table.hpp"
#include "string_pool.hpp"
#include "assembly_collection.hpp"

namespace HXSL
{
	inline void ASTNode::AssignId()
	{
		auto compilation = GetCompilation();
		HXSL_ASSERT(compilation != nullptr, "Cannot assign ID compilation was null.");
		id = compilation->GetNextID();
	}

	bool UsingDeclaration::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Target, AssemblyReferences);
		return AssemblyReferences.size() > 0;
	}

	void SymbolDef::SetAssembly(const Assembly* assembly, size_t tableIndex)
	{
		assembly = assembly;
		auto table = assembly->GetSymbolTable();
		tableIndex = tableIndex;
		fullyQualifiedName = std::make_unique<std::string>(table->GetFullyQualifiedName(tableIndex).c_str());
		if (isExtern)
		{
			std::string_view fqnView = *fullyQualifiedName.get();
			auto pos = fqnView.rfind(QUALIFIER_SEP);
			if (pos != std::string::npos)
			{
				pos++;
				name = TextSpan(fqnView.data(), pos, fqnView.size() - pos, 0, 0);
			}
			else
			{
				name = TextSpan(fqnView.data(), 0, fqnView.size(), 0, 0);
			}
		}
	}

	const SymbolMetadata* SymbolDef::GetMetadata() const
	{
		return GetTable()->GetNode(tableIndex).Metadata.get();
	}

	void SymbolRef::SetTable(const SymbolTable* table, size_t tableIndex)
	{
		this->table = table;
		this->tableIndex = tableIndex;
		auto meta = GetMetadata();
		GetDeclaration()->AddRef(this);
		fullyQualifiedName = std::make_unique<std::string>(table->GetFullyQualifiedName(tableIndex).c_str());
	}

	const std::string& SymbolRef::GetFullyQualifiedName() const
	{
		return *fullyQualifiedName.get();
	}

	const SymbolMetadata* SymbolRef::GetMetadata() const
	{
		return table->GetNode(tableIndex).Metadata.get();
	}

	SymbolDef* SymbolRef::GetDeclaration() const
	{
		if (table == nullptr) return nullptr;
		return table->GetNode(tableIndex).Metadata.get()->declaration;
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
	}

#define UNIQUE_PTR_CAST(ptr, type) \
std::move(std::unique_ptr<type>(static_cast<type*>(std::move(ptr).release())))

	void Struct::Write(Stream& stream) const
	{
	}

	void Struct::Read(Stream& stream, StringPool& container)
	{
	}

	void Struct::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], Field));
				break;
			case SymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], Function));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			}
		}
	}

	void Parameter::Write(Stream& stream) const
	{
		stream.WriteUInt(flags);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Parameter::Read(Stream& stream, StringPool& container)
	{
		flags = static_cast<ParameterFlags>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		symbol = std::make_unique<SymbolRef>();
		symbol->Read(stream);
	}

	void Parameter::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
	}

	void Field::Write(Stream& stream) const
	{
		stream.WriteUInt(flags);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Field::Read(Stream& stream, StringPool& container)
	{
		flags = static_cast<FieldFlags>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		symbol = std::make_unique<SymbolRef>();
		symbol->Read(stream);
	}

	void Field::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
	}

	void Function::Write(Stream& stream) const
	{
		stream.WriteUInt(flags);
		stream.WriteString(semantic);
		returnSymbol->Write(stream);
	}

	void Function::Read(Stream& stream, StringPool& container)
	{
		flags = static_cast<FunctionFlags>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		returnSymbol = std::make_unique<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void Function::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case NodeType_Parameter:
				AddParameter(UNIQUE_PTR_CAST(nodes[childIdx], Parameter));
				break;
			}
		}
	}

	void DeclarationStatement::Write(Stream& stream) const
	{
		//HXSL_ASSERT(false, "Cannot write declaration statements.");
	}

	void DeclarationStatement::Read(Stream& stream, StringPool& container)
	{
		//HXSL_ASSERT(false, "Cannot read declaration statements.");
	}

	void DeclarationStatement::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		//HXSL_ASSERT(false, "Cannot build declaration statements.");
	}

	void Class::Write(Stream& stream) const
	{
	}

	void Class::Read(Stream& stream, StringPool& container)
	{
	}

	void Class::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], Field));
				break;
			case SymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], Function));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			}
		}
	}

	void Namespace::Write(Stream& stream) const
	{
	}

	void Namespace::Read(Stream& stream, StringPool& container)
	{
	}

	void Namespace::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Field:
				AddField(UNIQUE_PTR_CAST(nodes[childIdx], Field));
				break;
			case SymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], Function));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			}
		}
	}

	void Namespace::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(name, this->references);
	}

	void Container::AddFunction(std::unique_ptr<Function> function)
	{
		function->SetParent(this);
		functions.push_back(std::move(function));
	}

	void Container::AddStruct(std::unique_ptr<Struct> _struct)
	{
		_struct->SetParent(this);
		structs.push_back(std::move(_struct));
	}

	void Container::AddClass(std::unique_ptr<Class> _class)
	{
		_class->SetParent(this);
		classes.push_back(std::move(_class));
	}

	void Container::AddField(std::unique_ptr<Field> field)
	{
		field->SetParent(this);
		fields.push_back(std::move(field));
	}
}