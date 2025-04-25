#include "ast.hpp"
#include "symbols/symbol_table.hpp"
#include "symbols/symbol_resolver.hpp"
#include "string_pool.hpp"
#include "assembly_collection.hpp"

namespace HXSL
{
	inline void ASTNode::AssignId()
	{
		if (id != 0) return;
		auto compilation = GetCompilation();
		if (!compilation) return;
		//HXSL_ASSERT(compilation != nullptr, "Cannot assign ID compilation was null.");
		id = compilation->GetNextID();
		for (auto child : children)
		{
			child->AssignId();
		}
	}

	bool UsingDeclaration::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Target, AssemblyReferences);
		return AssemblyReferences.size() > 0;
	}

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
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], FunctionOverload));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			case SymbolType_Class:
				AddClass(UNIQUE_PTR_CAST(nodes[childIdx], Class));
				break;
			case SymbolType_Operator:
				AddOperator(UNIQUE_PTR_CAST(nodes[childIdx], OperatorOverload));
				break;
			}
		}
	}

	void Parameter::Write(Stream& stream) const
	{
		stream.WriteUInt(flags);
		stream.WriteUInt(interpolationModifiers);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Parameter::Read(Stream& stream, StringPool& container)
	{
		flags = static_cast<ParameterFlags>(stream.ReadUInt());
		interpolationModifiers = static_cast<InterpolationModifier>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		symbol = std::make_unique<SymbolRef>();
		symbol->Read(stream);
	}

	void Parameter::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
	}

	void Field::Write(Stream& stream) const
	{
		stream.WriteUInt(storageClass);
		stream.WriteUInt(interpolationModifiers);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Field::Read(Stream& stream, StringPool& container)
	{
		storageClass = static_cast<StorageClass>(stream.ReadUInt());
		interpolationModifiers = static_cast<InterpolationModifier>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		symbol = std::make_unique<SymbolRef>();
		symbol->Read(stream);
	}

	void Field::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
	}

	void FunctionOverload::Write(Stream& stream) const
	{
		stream.WriteUInt(functionFlags);
		stream.WriteString(semantic);
		returnSymbol->Write(stream);
	}

	void FunctionOverload::Read(Stream& stream, StringPool& container)
	{
		functionFlags = static_cast<FunctionFlags>(stream.ReadUInt());
		semantic = TextSpan(container.add(stream.ReadString()));
		returnSymbol = std::make_unique<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void FunctionOverload::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Parameter:
				AddParameter(UNIQUE_PTR_CAST(nodes[childIdx], Parameter));
				break;
			}
		}
	}

	void OperatorOverload::Write(Stream& stream) const
	{
		stream.WriteUInt(functionFlags);
		stream.WriteUInt(operatorFlags);
		stream.WriteUInt(_operator);
		returnSymbol->Write(stream);
	}

	void OperatorOverload::Read(Stream& stream, StringPool& container)
	{
		functionFlags = static_cast<FunctionFlags>(stream.ReadUInt());
		operatorFlags = static_cast<OperatorFlags>(stream.ReadUInt());
		_operator = static_cast<Operator>(stream.ReadUInt());
		returnSymbol = std::make_unique<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void OperatorOverload::Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes)
	{
		FunctionOverload::Build(table, index, compilation, nodes);
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
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], FunctionOverload));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			case SymbolType_Class:
				AddClass(UNIQUE_PTR_CAST(nodes[childIdx], Class));
				break;
			case SymbolType_Operator:
				AddOperator(UNIQUE_PTR_CAST(nodes[childIdx], OperatorOverload));
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
				AddFunction(UNIQUE_PTR_CAST(nodes[childIdx], FunctionOverload));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST(nodes[childIdx], Struct));
				break;
			case SymbolType_Class:
				AddClass(UNIQUE_PTR_CAST(nodes[childIdx], Class));
				break;
			case SymbolType_Operator:
				AddOperator(UNIQUE_PTR_CAST(nodes[childIdx], OperatorOverload));
				break;
			}
		}
	}

	void Namespace::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(name, this->references);
	}

	void Container::AddFunction(std::unique_ptr<FunctionOverload> function)
	{
		function->SetParent(this);
		functions.push_back(std::move(function));
	}

	void Container::AddOperator(std::unique_ptr<OperatorOverload> _operator)
	{
		_operator->SetParent(this);
		operators.push_back(std::move(_operator));
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

	void ReturnStatement::TypeCheck(SymbolResolver& resolver)
	{
		auto func = FindAncestor<FunctionOverload>(NodeType_FunctionOverload);
		SymbolDef* retType = func->GetReturnSymbolRef()->GetDeclaration();
		SymbolDef* exprType = GetReturnValueExpression()->GetInferredType();
		if (!retType->IsEquivalentTo(exprType))
		{
			resolver.LogError("Return type mismatch: expected '%s' but got '%s'", GetSpan(), retType->GetFullyQualifiedName().c_str(), exprType->GetFullyQualifiedName().c_str());
		}
	}
}