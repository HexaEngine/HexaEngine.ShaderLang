#include "pointer.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Pointer* Pointer::Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<Pointer>(sizeof(Pointer), span, name, elementType);
	}
}