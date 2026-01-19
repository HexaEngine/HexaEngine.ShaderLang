#include "primitive.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Primitive* Primitive::Create(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, Span<OperatorOverload*>& operators)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<Primitive>(TotalSizeToAlloc(operators.size()), span, name, kind, _class, rows, columns);
		ptr->storage.InitializeMove(ptr, operators);
		return ptr;
	}

	Primitive* Primitive::Create(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, uint32_t numOperators)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<Primitive>(TotalSizeToAlloc(numOperators), span, name, kind, _class, rows, columns);
		ptr->storage.SetCounts(numOperators);
		return ptr;
	}

	void Primitive::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetOperators);
	}

	void Primitive::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetOperators);
	}
}