#ifndef AST_HELPERS_HPP
#define AST_HELPERS_HPP

#include "declarations.hpp"
#include "statements.hpp"
#include "expressions.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "pointer.hpp"
#include "swizzle.hpp"

namespace HXSL
{
	struct OperatorHelper
	{
		static Span<OperatorOverload*> TryGetOperators(ASTNode* node)
		{
			if (auto x = dyn_cast<Class>(node)) { return x->GetOperators(); }
			if (auto x = dyn_cast<Struct>(node)) { return x->GetOperators(); }
			if (auto x = dyn_cast<Primitive>(node)) { return x->GetOperators(); }
			return {};
		}
	};

	struct SymbolRefHelper
	{
		static SymbolRef* TryGetSymbolRef(ASTNode* node);

		static SymbolRef* GetSymbolRef(ASTNode* node)
		{
			auto p = TryGetSymbolRef(node);
			if (p == nullptr) assert(false);
			return p;
		}
	};
}

#endif