#include "container.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation_unit.hpp"

namespace HXSL
{
	void Container::AddFunction(ast_ptr<FunctionOverload> function)
	{
		function->SetParent(this);
		functions.push_back(std::move(function));
	}

	void Container::AddOperator(ast_ptr<OperatorOverload> _operator)
	{
		_operator->SetParent(this);
		operators.push_back(std::move(_operator));
	}

	void Container::AddStruct(ast_ptr<Struct> _struct)
	{
		_struct->SetParent(this);
		structs.push_back(std::move(_struct));
	}

	void Container::AddClass(ast_ptr<Class> _class)
	{
		_class->SetParent(this);
		classes.push_back(std::move(_class));
	}

	void Container::AddField(ast_ptr<Field> field)
	{
		field->SetParent(this);
		fields.push_back(std::move(field));
	}

	void Container::TransferContentsTo(Container& target, ContainerTransferFlags flags)
	{
		auto move_all_with_parent = [&target](auto& from, auto& to) {
			for (auto& item : from)
			{
				item->SetCanonicalParent(item->GetParent());
				item->SetParent(&target);
				to.push_back(std::move(item));
			}
			from.clear();
			};

		if ((flags & ContainerTransferFlags_Functions) != 0) move_all_with_parent(functions, target.GetFunctionsMut());
		if ((flags & ContainerTransferFlags_Operators) != 0) move_all_with_parent(operators, target.GetOperatorsMut());
		if ((flags & ContainerTransferFlags_Structs) != 0) move_all_with_parent(structs, target.GetStructsMut());
		if ((flags & ContainerTransferFlags_Classes) != 0) move_all_with_parent(classes, target.GetClassesMut());
		if ((flags & ContainerTransferFlags_Fields) != 0) move_all_with_parent(fields, target.GetFieldsMut());
	}
}