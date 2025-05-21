#include "container.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation.hpp"

namespace HXSL
{
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

	void Container::TransferContentsTo(Container& target)
	{
		auto move_all_with_parent = [&target](auto& from, auto& to) {
			for (auto& item : from)
			{
				item->SetParent(&target);
				to.push_back(std::move(item));
			}
			from.clear();
			};

		move_all_with_parent(functions, target.GetFunctionsMut());
		move_all_with_parent(operators, target.GetOperatorsMut());
		move_all_with_parent(structs, target.GetStructsMut());
		move_all_with_parent(classes, target.GetClassesMut());
		move_all_with_parent(fields, target.GetFieldsMut());
	}
}