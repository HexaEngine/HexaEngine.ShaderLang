#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"
#include "symbol_base.hpp"

namespace HXSL
{
	class TypeContainer : public Type, public IHasOperatorOverloads
	{
	protected:
		std::vector<ast_ptr<FunctionOverload>> functions;
		std::vector<ast_ptr<ConstructorOverload>> constructors;
		std::vector<ast_ptr<OperatorOverload>> operators;
		std::vector<ast_ptr<Struct>> structs;
		std::vector<ast_ptr<Class>> classes;
		std::vector<ast_ptr<Field>> fields;

	public:
		TypeContainer(TextSpan span, NodeType type, TextSpan name, AccessModifier accessModifiers, bool isExtern = false)
			: Type(span, type, name, accessModifiers, isExtern)
		{
		}

		TypeContainer(TextSpan span, NodeType type, const std::string& name, AccessModifier accessModifiers, bool isExtern = false)
			: Type(span, type, name, accessModifiers, isExtern)
		{
		}

	public:

		virtual ~TypeContainer() {}

		void AddFunction(ast_ptr<FunctionOverload> function)
		{
			RegisterChild(function);
			functions.push_back(std::move(function));
		}

		void AddConstructor(ast_ptr<ConstructorOverload> constructor)
		{
			RegisterChild(constructor);
			constructors.push_back(std::move(constructor));
		}

		void AddOperator(ast_ptr<OperatorOverload> _operator)
		{
			RegisterChild(_operator);
			operators.push_back(std::move(_operator));
		}

		void AddStruct(ast_ptr<Struct> _struct)
		{
			RegisterChild(_struct);
			structs.push_back(std::move(_struct));
		}

		void AddClass(ast_ptr<Class> _class)
		{
			RegisterChild(_class);
			classes.push_back(std::move(_class));
		}

		void AddField(ast_ptr<Field> field)
		{
			RegisterChild(field);
			fields.push_back(std::move(field));
		}

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<FunctionOverload>>, Functions, functions);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<ConstructorOverload>>, Constructors, constructors);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Struct>>, Structs, structs);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Class>>, Classes, classes);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Field>>, Fields, fields);

		const std::vector<ast_ptr<OperatorOverload>>& GetOperators() const noexcept override
		{
			return operators;
		}

		std::vector<ast_ptr<OperatorOverload>>& GetOperatorsMut() noexcept
		{
			return operators;
		}
	};
}

#endif