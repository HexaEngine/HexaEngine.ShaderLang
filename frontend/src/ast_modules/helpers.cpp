#include "helpers.hpp"

namespace HXSL
{
	SymbolRef* SymbolRefHelper::TryGetSymbolRef(ASTNode* node)
	{
		if (auto x = dyn_cast<Parameter>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<Field>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<ThisDef>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<DeclarationStatement>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<MemberReferenceExpression>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<FunctionCallExpression>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<MemberAccessExpression>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<IndexerAccessExpression>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<ArrayDecl>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<Pointer>(node)) { return x->GetSymbolRef(); }
		if (auto x = dyn_cast<SwizzleDefinition>(node)) { return x->GetSymbolRef(); }
		return nullptr;
	}
}
