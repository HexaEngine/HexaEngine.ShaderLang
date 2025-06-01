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
		static ArrayRef<ast_ptr<OperatorOverload>> TryGetOperators(ASTNode* node)
		{
			if (auto x = dyn_cast<Class>(node)) { return x->GetOperators(); }
			if (auto x = dyn_cast<Struct>(node)) { return x->GetOperators(); }
			if (auto x = dyn_cast<Primitive>(node)) { return x->GetOperators(); }
			return {};
		}
	};

	struct SymbolRefHelper
	{
		static const ast_ptr<SymbolRef>* TryGetSymbolRef(ASTNode* node)
		{
			if (auto x = dyn_cast<Parameter>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<Field>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<ThisDef>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<DeclarationStatement>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<MemberReferenceExpression>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<FunctionCallExpression>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<MemberAccessExpression>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<IndexerAccessExpression>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<Array>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<Pointer>(node)) { return &x->GetSymbolRef(); }
			if (auto x = dyn_cast<SwizzleDefinition>(node)) { return &x->GetSymbolRef(); }
			return nullptr;
		}

		static const ast_ptr<SymbolRef>& GetSymbolRef(ASTNode* node)
		{
			auto p = TryGetSymbolRef(node);
			if (p == nullptr) assert(false);
			return *p;
		}
	};
}

#endif