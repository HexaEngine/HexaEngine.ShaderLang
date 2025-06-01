#include "pointer.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Pointer* Pointer::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType)
	{
		return context->Alloc<Pointer>(sizeof(Pointer), span, name, std::move(elementType));
	}
}