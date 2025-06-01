#include "swizzle.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	SwizzleDefinition* SwizzleDefinition::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, uint8_t mask, Primitive* basePrim, ast_ptr<SymbolRef>&& symbol)
	{
		return context->Alloc<SwizzleDefinition>(sizeof(SwizzleDefinition), span, name, mask, basePrim, std::move(symbol));
	}
}