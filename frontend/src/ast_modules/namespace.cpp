#include "namespace.hpp"
#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "il/assembly_collection.hpp"

namespace HXSL
{
	bool UsingDeclaration::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Target->name, AssemblyReferences);
		return AssemblyReferences.size() > 0;
	}

	Namespace* Namespace::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name,
		ArrayRef<ast_ptr<Struct>> structs,
		ArrayRef<ast_ptr<Class>> classes,
		ArrayRef<ast_ptr<FunctionOverload>> functions,
		ArrayRef<ast_ptr<Field>> fields,
		ArrayRef<ast_ptr<Namespace>> nestedNamespaces,
		ArrayRef<UsingDeclaration> usings)
	{
		auto ptr = context->Alloc<Namespace>(TotalSizeToAlloc(structs.size(), classes.size(), functions.size(), fields.size(), nestedNamespaces.size(), usings.size()), span, name);
		ptr->numStructs = static_cast<uint32_t>(structs.size());
		ptr->numClasses = static_cast<uint32_t>(classes.size());
		ptr->numFunctions = static_cast<uint32_t>(functions.size());
		ptr->numFields = static_cast<uint32_t>(fields.size());
		ptr->numNestedNamespaces = static_cast<uint32_t>(nestedNamespaces.size());
		ptr->numUsings = static_cast<uint32_t>(usings.size());
		std::uninitialized_move(structs.begin(), structs.end(), ptr->GetStructs().data());
		std::uninitialized_move(classes.begin(), classes.end(), ptr->GetClasses().data());
		std::uninitialized_move(functions.begin(), functions.end(), ptr->GetFunctions().data());
		std::uninitialized_move(fields.begin(), fields.end(), ptr->GetFields().data());
		std::uninitialized_move(nestedNamespaces.begin(), nestedNamespaces.end(), ptr->GetNestedNamespacess().data());
		std::uninitialized_move(usings.begin(), usings.end(), ptr->GetUsings().data());
		return ptr;
	}

	void Namespace::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(GetName(), this->GetAssemblyReferences());
	}
	/*
	void Namespace::Write(Stream& stream) const
	{
	}

	void Namespace::Read(Stream& stream, StringPool& container)
	{
	}

	void Namespace::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Field:
				AddField(UNIQUE_PTR_CAST_AST(nodes[childIdx], Field));
				break;
			case SymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST_AST(nodes[childIdx], FunctionOverload));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST_AST(nodes[childIdx], Struct));
				break;
			case SymbolType_Class:
				AddClass(UNIQUE_PTR_CAST_AST(nodes[childIdx], Class));
				break;
			}
		}
	}
	*/
}