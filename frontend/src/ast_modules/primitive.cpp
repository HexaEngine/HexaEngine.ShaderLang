#include "primitive.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Primitive* Primitive::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, ArrayRef<ast_ptr<OperatorOverload>>& operators)
	{
		auto ptr = context->Alloc<Primitive>(TotalSizeToAlloc(operators.size()), span, name, kind, _class, rows, columns);
		ptr->numOperators = static_cast<uint32_t>(operators.size());
		std::uninitialized_move(operators.begin(), operators.end(), ptr->GetOperators().data());
		return ptr;
	}

	Primitive* Primitive::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, uint32_t numOperators)
	{
		auto ptr = context->Alloc<Primitive>(TotalSizeToAlloc(numOperators), span, name, kind, _class, rows, columns);
		ptr->numOperators = numOperators;
		ptr->GetOperators().init();
		return ptr;
	}
}