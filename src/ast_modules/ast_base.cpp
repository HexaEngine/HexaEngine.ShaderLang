#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation.hpp"

namespace HXSL
{
	void ASTNode::AssignId()
	{
		if (id != 0) return;
		auto compilation = GetCompilation();
		if (!compilation) return;
		//HXSL_ASSERT(compilation != nullptr, "Cannot assign ID compilation was null.");
		id = compilation->GetNextID();
		for (auto child : children)
		{
			child->AssignId();
		}
	}
}