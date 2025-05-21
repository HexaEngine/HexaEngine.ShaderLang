#include "expressions.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"

namespace HXSL
{
	bool Expression::IsVoidType() const noexcept
	{
		if (!inferredType) return false;
		auto prim = inferredType->As<Primitive>();
		if (prim)
		{
			return prim->GetKind() == PrimitiveKind_Void;
		}
		return false;
	}
}