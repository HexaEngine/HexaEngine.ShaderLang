#include "namespace.hpp"
#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "il/assembly_collection.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	UsingDecl* UsingDecl::Create(const TextSpan& span, IdentifierInfo* name, IdentifierInfo* alias)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<UsingDecl>(sizeof(UsingDecl), span, name, alias);
	}

	bool UsingDecl::Warmup(const AssemblyCollection& references)
	{
		auto* context = ASTContext::GetCurrentContext();
		std::vector<AssemblySymbolRef> refs;
		references.FindAssembliesByNamespace(target->name, refs);
		assemblyReferences = context->AllocCopy<AssemblySymbolRef>(refs);
		return assemblyReferences.size() > 0;
	}

	Namespace* Namespace::Create(const TextSpan& span, IdentifierInfo* name,
		const ArrayRef<Struct*>& structs,
		const ArrayRef<Class*>& classes,
		const ArrayRef<FunctionOverload*>& functions,
		const ArrayRef<Field*>& fields,
		const ArrayRef<Namespace*>& nestedNamespaces,
		const ArrayRef<UsingDecl*>& usings)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<Namespace>(TotalSizeToAlloc(structs.size(), classes.size(), functions.size(), fields.size(), nestedNamespaces.size(), usings.size()), span, name);
		ptr->storage.InitializeMove(ptr, structs, classes, functions, fields, nestedNamespaces, usings);
		REGISTER_CHILDREN_PTR(ptr, GetStructs());
		REGISTER_CHILDREN_PTR(ptr, GetClasses());
		REGISTER_CHILDREN_PTR(ptr, GetFunctions());
		REGISTER_CHILDREN_PTR(ptr, GetFields());
		REGISTER_CHILDREN_PTR(ptr, GetUsings());
		return ptr;
	}

	void Namespace::Warmup(const AssemblyCollection& references)
	{
		auto* context = ASTContext::GetCurrentContext();
		std::vector<AssemblySymbolRef> refs;
		references.FindAssembliesByNamespace(GetName(), refs);
		assemblyReferences = context->AllocCopy<AssemblySymbolRef>(refs);
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