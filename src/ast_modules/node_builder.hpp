#ifndef NODE_BUILDER_HPP
#define NODE_BUILDER_HPP

#include "pch/ast.hpp"
#include "il/assembly.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	class ASTNodeBuilder
	{
	protected:
		Assembly* assembly;
		SymbolTable* table;

		void ResolveInternal(SymbolRef* ref)
		{
			auto handle = table->FindNodeIndexPart(ref->GetName());
			if (handle.invalid())
			{
				HXSL_ASSERT(false, "Couldn't find type.");
			}
			ref->SetTable(handle);
		}

		ASTNodeBuilder(Assembly* assembly) : assembly(assembly), table(assembly->GetMutableSymbolTable())
		{
		}
	};

	class FunctionBuilder : ASTNodeBuilder
	{
		std::unique_ptr<FunctionOverload> func;
		std::vector<std::unique_ptr<Parameter>> parameters;
	public:
		FunctionBuilder(Assembly* assembly) : ASTNodeBuilder(assembly), func(std::make_unique<FunctionOverload>())
		{
		}

		FunctionBuilder& WithName(const std::string& name)
		{
			func->SetName(name);
			return *this;
		}

		FunctionBuilder& WithParam(const std::string& paramName, const std::string& paramType)
		{
			auto param = std::make_unique<Parameter>();
			param->SetName(paramName);

			auto paramRef = std::make_unique<SymbolRef>(paramType, SymbolRefType_Type, false);
			ResolveInternal(paramRef.get());
			param->SetSymbolRef(std::move(paramRef));

			parameters.push_back(std::move(param));
			return *this;
		}

		FunctionBuilder& Returns(const std::string& returnType)
		{
			auto returnRef = std::make_unique<SymbolRef>(returnType, SymbolRefType_Type, false);
			ResolveInternal(returnRef.get());
			func->SetReturnSymbol(std::move(returnRef));
			return *this;
		}

		void AttachToClass(Class* _class)
		{
			AttachToContainer(_class, _class->GetSymbolHandle());
		}

		void AttachToStruct(Struct* _struct)
		{
			AttachToContainer(_struct, _struct->GetSymbolHandle());
		}

		void AttachToContainer(Container* container, SymbolHandle handle)
		{
			auto funcPtr = func.get();
			container->AddFunction(std::move(func));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				funcPtr->AddParameter(std::move(parameters[i]));
			}

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Function, SymbolScopeType_Class, AccessModifier_Public, 0, funcPtr);
			auto signature = funcPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(signature, meta, handle.GetIndex());
			funcPtr->SetAssembly(assembly, funcIndex);
		}
	};

	class OperatorBuilder : ASTNodeBuilder
	{
		std::unique_ptr<OperatorOverload> _operator;
		std::vector<std::unique_ptr<Parameter>> parameters;
	public:
		OperatorBuilder(Assembly* assembly) : ASTNodeBuilder(assembly), _operator(std::make_unique<OperatorOverload>())
		{
		}

		OperatorBuilder& WithOp(const OperatorFlags& flags, const Operator& op)
		{
			_operator->SetOperatorFlags(flags);
			_operator->SetOperator(op);
			return *this;
		}

		OperatorBuilder& WithParam(const std::string& paramName, const std::string& paramType)
		{
			auto param = std::make_unique<Parameter>();
			param->SetName(paramName);

			auto paramRef = std::make_unique<SymbolRef>(paramType, SymbolRefType_Type, false);
			param->SetSymbolRef(std::move(paramRef));

			parameters.push_back(std::move(param));
			return *this;
		}

		OperatorBuilder& Returns(const std::string& returnType)
		{
			auto returnRef = std::make_unique<SymbolRef>(returnType, SymbolRefType_Type, false);
			_operator->SetReturnSymbol(std::move(returnRef));
			return *this;
		}

		void AttachToClass(Class* _class)
		{
			AttachToContainer(_class, _class->GetSymbolHandle());
		}

		void AttachToStruct(Struct* _struct)
		{
			AttachToContainer(_struct, _struct->GetSymbolHandle());
		}

		void AttachToContainer(Container* container, SymbolHandle handle)
		{
			auto operatorPtr = _operator.get();
			auto& op = _operator->GetOperator();
			std::string name = _operator->GetOperator() == Operator_Cast ? "operator" : "operator" + ToString(op);
			operatorPtr->SetName(name);
			container->AddOperator(std::move(_operator));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				operatorPtr->AddParameter(std::move(parameters[i]));
			}

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Operator, SymbolScopeType_Class, AccessModifier_Public, 0, operatorPtr);
			auto signature = operatorPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(signature, meta, handle.GetIndex());
			operatorPtr->SetAssembly(assembly, funcIndex);
		}

		void AttachToPrimitive(Primitive* container)
		{
			auto operatorPtr = _operator.get();
			auto& op = _operator->GetOperator();
			std::string name = _operator->GetOperator() == Operator_Cast ? "operator" : "operator" + ToString(op);
			operatorPtr->SetName(name);
			container->AddOperator(std::move(_operator));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				ResolveInternal(parameters[i]->GetSymbolRef().get());
				operatorPtr->AddParameter(std::move(parameters[i]));
			}
			ResolveInternal(operatorPtr->GetReturnSymbolRef().get());
			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Operator, SymbolScopeType_Struct, AccessModifier_Public, 0, operatorPtr);
			auto signature = operatorPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(signature, meta, container->GetSymbolHandle().GetIndex());
			operatorPtr->SetAssembly(assembly, funcIndex);
		}
	};

	class ClassBuilder : ASTNodeBuilder
	{
		std::unique_ptr<Class> _class;
		std::vector<std::unique_ptr<FunctionBuilder>> functions;
		std::vector<std::unique_ptr<Field>> fields;
		std::vector<std::unique_ptr<OperatorBuilder>> operators;
	public:
		ClassBuilder(Assembly* assembly) : ASTNodeBuilder(assembly), _class(std::make_unique<Class>())
		{
		}

		ClassBuilder& WithName(const std::string& name)
		{
			_class->SetName(name);
			return *this;
		}

		FunctionBuilder& WithFunction()
		{
			auto builder = std::make_unique<FunctionBuilder>(assembly);
			auto ptr = builder.get();
			functions.push_back(std::move(builder));
			return *ptr;
		}

		ClassBuilder& WithField(const std::string& name, const std::string& type)
		{
			auto param = std::make_unique<Field>();
			param->SetName(name);

			auto paramRef = std::make_unique<SymbolRef>(type, SymbolRefType_Type, false);
			ResolveInternal(paramRef.get());
			param->SetSymbolRef(std::move(paramRef));

			fields.push_back(std::move(param));
			return *this;
		}

		OperatorBuilder& WithOperator(const std::string& name, const std::string& type)
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);
			auto ptr = builder.get();
			operators.push_back(std::move(builder));
			return *ptr;
		}
	};

	class PrimitiveBuilder : ASTNodeBuilder
	{
		std::unique_ptr<Primitive> primitive;
		std::vector<std::unique_ptr<OperatorBuilder>> operators;
		Primitive* prim;
	public:
		PrimitiveBuilder(Assembly* assembly) : ASTNodeBuilder(assembly), primitive(std::make_unique<Primitive>()), prim(nullptr)
		{
		}

		PrimitiveBuilder& WithName(const std::string& name)
		{
			primitive->SetName(name);
			return *this;
		}

		PrimitiveBuilder& WithKind(PrimitiveKind kind, PrimitiveClass _class)
		{
			primitive->SetKind(kind);
			primitive->SetClass(_class);
			return *this;
		}

		PrimitiveBuilder& WithRowsAndColumns(uint32_t rows, uint32_t columns)
		{
			primitive->SetRows(rows);
			primitive->SetColumns(columns);
			return *this;
		}

		OperatorBuilder& WithOperator()
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);
			auto ptr = builder.get();
			operators.push_back(std::move(builder));
			return *ptr;
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& str, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(flags, op)
					.WithParam("left", str)
					.WithParam("right", str)
					.Returns(str);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& strA, const std::string& strB, const std::string& strRet, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(flags, op)
					.WithParam("left", strA)
					.WithParam("right", strB)
					.Returns(strRet);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(flags, op)
					.WithParam("value", str)
					.Returns(str);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(OperatorFlags_None, op)
					.WithParam("value", str)
					.Returns(strRet);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithImplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);

			builder->WithOp(OperatorFlags_Implicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			auto ptr = builder.get();
			operators.push_back(std::move(builder));
		}

		void WithExplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);

			builder->WithOp(OperatorFlags_Explicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			auto ptr = builder.get();
			operators.push_back(std::move(builder));
		}

		void Finish()
		{
			auto compilation = assembly->GetMutableSymbolTable()->GetCompilation();
			auto primitivePtr = primitive.get();
			compilation->primitives.push_back(std::move(primitive));

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Primitive, SymbolScopeType_Global, AccessModifier_Public, 0, primitivePtr);
			auto index = table->Insert(primitivePtr->GetName(), meta, 0);
			primitivePtr->SetAssembly(assembly, index);

			for (size_t i = 0; i < operators.size(); i++)
			{
				operators[i]->AttachToPrimitive(primitivePtr);
			}
		}

		bool FinishPhased()
		{
			if (prim == nullptr)
			{
				auto compilation = assembly->GetMutableSymbolTable()->GetCompilation();
				prim = primitive.get();
				compilation->primitives.push_back(std::move(primitive));

				auto meta = std::make_shared<SymbolMetadata>(SymbolType_Primitive, SymbolScopeType_Global, AccessModifier_Public, 0, prim);
				auto index = table->Insert(prim->GetName(), meta, 0);
				prim->SetAssembly(assembly, index);
				return false;
			}
			else
			{
				for (size_t i = 0; i < operators.size(); i++)
				{
					operators[i]->AttachToPrimitive(prim);
				}
				operators.clear();
				return true;
			}
		}
	};
}

#endif