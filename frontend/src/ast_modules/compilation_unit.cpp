#include "compilation_unit.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	CompilationUnit* CompilationUnit::Create(bool isExtern, const Span<Namespace*>& namespaces, const Span<UsingDecl*>& usings)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<CompilationUnit>(TotalSizeToAlloc(namespaces.size(), usings.size()), isExtern);
		ptr->storage.InitializeMove(ptr, namespaces, usings);
		REGISTER_CHILDREN_PTR(ptr, GetNamespaces());
		REGISTER_CHILDREN_PTR(ptr, GetUsings());
		return ptr;
	}

	CompilationUnit* CompilationUnit::Create(bool isExtern, uint32_t numNamespaces, uint32_t numUsings)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<CompilationUnit>(TotalSizeToAlloc(numNamespaces, numUsings), isExtern);
		ptr->storage.SetCounts(numNamespaces, numUsings);
		return ptr;
	}

	void CompilationUnit::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetUsings);
		AST_ITERATE_CHILDREN_MUT(GetNamespaces);
	}

	void CompilationUnit::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetUsings);
		AST_ITERATE_CHILDREN(GetNamespaces);
	}
}