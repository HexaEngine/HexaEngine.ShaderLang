#include "compilation_unit.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	CompilationUnit* CompilationUnit::Create(ASTContext* context, bool isExtern, ArrayRef<ast_ptr<Namespace>> namespaces)
	{
		auto ptr = context->Alloc<CompilationUnit>(TotalSizeToAlloc(namespaces.size()), isExtern);
		ptr->numNamespaces = static_cast<uint32_t>(namespaces.size());
		std::uninitialized_move(namespaces.begin(), namespaces.end(), ptr->GetNamespaces().data());
		return ptr;
	}

	CompilationUnit* CompilationUnit::Create(ASTContext* context, bool isExtern, uint32_t numNamespaces)
	{
		auto ptr = context->Alloc<CompilationUnit>(TotalSizeToAlloc(numNamespaces), isExtern);
		ptr->numNamespaces = numNamespaces;
		ptr->GetNamespaces().init();
		return ptr;
	}
}