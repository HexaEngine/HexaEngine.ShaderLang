#include "swizzle.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	SwizzleDefinition* SwizzleDefinition::Create(const TextSpan& span, IdentifierInfo* name, uint8_t mask, Primitive* basePrim, SymbolRef* symbol)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<SwizzleDefinition>(sizeof(SwizzleDefinition), span, name, mask, basePrim, symbol);
	}
}