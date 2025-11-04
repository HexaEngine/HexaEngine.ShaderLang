#include "attributes.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	AttributeDecl* AttributeDecl::Create(const TextSpan& span, SymbolRef* symbol, const ArrayRef<Expression*>& parameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto* ptr = context->Alloc<AttributeDecl>(TotalSizeToAlloc(parameters.size()), span, symbol);
		ptr->storage.InitializeMove(ptr, parameters);
		return ptr;
	}

	AttributeDecl* AttributeDecl::Create(const TextSpan& span, SymbolRef* symbol, uint32_t numParameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto* ptr = context->Alloc<AttributeDecl>(TotalSizeToAlloc(numParameters), span, symbol);
		ptr->storage.SetCounts(numParameters);
		return ptr;
	}
}