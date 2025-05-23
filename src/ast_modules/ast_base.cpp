#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation_unit.hpp"

namespace HXSL
{
	void ASTNode::AssignId()
	{
		if (id != 0)
		{
			return;
		}

		auto compilation = GetCompilation(); // aka root.
		if (!compilation)
		{
			return;
		}

		//HXSL_ASSERT(compilation != nullptr, "Cannot assign ID compilation was null.");

		id = compilation->GetNextID();
		for (auto child : children)
		{
			child->AssignId();
		}
	}
}