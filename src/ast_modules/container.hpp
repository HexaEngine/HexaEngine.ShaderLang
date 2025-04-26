#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Container : virtual public ASTNode, public IHasOperators
	{
	protected:
		std::vector<std::unique_ptr<FunctionOverload>> functions;
		std::vector<std::unique_ptr<OperatorOverload>> operators;
		std::vector<std::unique_ptr<Struct>> structs;
		std::vector<std::unique_ptr<Class>> classes;
		std::vector<std::unique_ptr<Field>> fields;

	public:
		Container(TextSpan span, ASTNode* parent, NodeType type, bool isExtern = false)
			:ASTNode(span, parent, type, isExtern)
		{
		}
		virtual ~Container() {}
		void AddFunction(std::unique_ptr<FunctionOverload> function);

		void AddOperator(std::unique_ptr<OperatorOverload> _operator);

		void AddStruct(std::unique_ptr<Struct> _struct);

		void AddClass(std::unique_ptr<Class> _class);

		void AddField(std::unique_ptr<Field> field);

		const std::vector<std::unique_ptr<FunctionOverload>>& GetFunctions() const noexcept
		{
			return functions;
		}

		const std::vector<std::unique_ptr<OperatorOverload>>& GetOperators() const noexcept override
		{
			return operators;
		}

		const std::vector<std::unique_ptr<Struct>>& GetStructs() const noexcept
		{
			return structs;
		}

		const std::vector<std::unique_ptr<Class>>& GetClasses() const noexcept
		{
			return classes;
		}

		const std::vector<std::unique_ptr<Field>>& GetFields() const noexcept
		{
			return fields;
		}
	};
}

#endif