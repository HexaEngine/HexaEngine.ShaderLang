#include "array.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	ArrayDecl* ArrayDecl::Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType, size_t arraySize)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<ArrayDecl>(sizeof(ArrayDecl), span, name, elementType, arraySize);
	}
}