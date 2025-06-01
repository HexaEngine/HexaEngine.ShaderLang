#include "array.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Array* Array::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType, size_t arraySize)
	{
		return context->Alloc<Array>(sizeof(Array), span, name, std::move(elementType), arraySize);
	}
}