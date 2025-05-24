#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	enum ContainerTransferFlags : int
	{
		ContainerTransferFlags_None = 0,
		ContainerTransferFlags_Functions = 1 << 0,
		ContainerTransferFlags_Constructors = 1 << 1,
		ContainerTransferFlags_Operators = 1 << 2,
		ContainerTransferFlags_Structs = 1 << 3,
		ContainerTransferFlags_Classes = 1 << 4,
		ContainerTransferFlags_Enums = 1 << 5,
		ContainerTransferFlags_Fields = 1 << 6,
		ContainerTransferFlags_Default = ContainerTransferFlags_Functions | ContainerTransferFlags_Constructors | ContainerTransferFlags_Operators | ContainerTransferFlags_Structs | ContainerTransferFlags_Classes | ContainerTransferFlags_Enums,
		ContainerTransferFlags_All = ContainerTransferFlags_Default | ContainerTransferFlags_Fields,
	};

	DEFINE_FLAGS_OPERATORS(ContainerTransferFlags, int);

	class Container : virtual public ASTNode, public IHasOperatorOverloads
	{
	protected:
		std::vector<ast_ptr<FunctionOverload>> functions;
		std::vector<ast_ptr<ConstructorOverload>> constructors;
		std::vector<ast_ptr<OperatorOverload>> operators;
		std::vector<ast_ptr<Struct>> structs;
		std::vector<ast_ptr<Class>> classes;
		std::vector<ast_ptr<Field>> fields;

	public:
		Container(TextSpan span, NodeType type, bool isExtern = false) : ASTNode(span, type, isExtern)
		{
		}

	public:

		virtual ~Container() {}

		void AddFunction(ast_ptr<FunctionOverload> function);

		void AddConstructor(ast_ptr<ConstructorOverload> constructor);

		void AddOperator(ast_ptr<OperatorOverload> _operator);

		void AddStruct(ast_ptr<Struct> _struct);

		void AddClass(ast_ptr<Class> _class);

		void AddField(ast_ptr<Field> field);

		const std::vector<ast_ptr<FunctionOverload>>& GetFunctions() const noexcept
		{
			return functions;
		}

		const std::vector<ast_ptr<ConstructorOverload>>& GetConstructors() const noexcept
		{
			return constructors;
		}

		const std::vector<ast_ptr<OperatorOverload>>& GetOperators() const noexcept override
		{
			return operators;
		}

		const std::vector<ast_ptr<Struct>>& GetStructs() const noexcept
		{
			return structs;
		}

		const std::vector<ast_ptr<Class>>& GetClasses() const noexcept
		{
			return classes;
		}

		const std::vector<ast_ptr<Field>>& GetFields() const noexcept
		{
			return fields;
		}

		std::vector<ast_ptr<FunctionOverload>>& GetFunctionsMut() noexcept
		{
			return functions;
		}

		std::vector<ast_ptr<ConstructorOverload>>& GetConstructorsMut() noexcept
		{
			return constructors;
		}

		std::vector<ast_ptr<OperatorOverload>>& GetOperatorsMut() noexcept
		{
			return operators;
		}

		std::vector<ast_ptr<Struct>>& GetStructsMut() noexcept
		{
			return structs;
		}

		std::vector<ast_ptr<Class>>& GetClassesMut() noexcept
		{
			return classes;
		}

		std::vector<ast_ptr<Field>>& GetFieldsMut() noexcept
		{
			return fields;
		}

		void TransferContentsTo(Container& target, ContainerTransferFlags flags);
	};
}

#endif