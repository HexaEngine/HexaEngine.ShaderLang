#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "compilation.hpp"

namespace HXSL
{
	inline void ASTNode::AssignId()
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

	void Container::AddFunction(std::unique_ptr<FunctionOverload> function)
	{
		function->SetParent(this);
		functions.push_back(std::move(function));
	}

	void Container::AddOperator(std::unique_ptr<OperatorOverload> _operator)
	{
		_operator->SetParent(this);
		operators.push_back(std::move(_operator));
	}

	void Container::AddStruct(std::unique_ptr<Struct> _struct)
	{
		_struct->SetParent(this);
		structs.push_back(std::move(_struct));
	}

	void Container::AddClass(std::unique_ptr<Class> _class)
	{
		_class->SetParent(this);
		classes.push_back(std::move(_class));
	}

	void Container::AddField(std::unique_ptr<Field> field)
	{
		field->SetParent(this);
		fields.push_back(std::move(field));
	}
}