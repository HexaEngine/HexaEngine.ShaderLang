#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "config.h"
#include "ast.hpp"
#include "text_span.h"
#include "symbols/symbol_table.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace HXSL
{
	class ASTNodeBuilder
	{
	protected:
		Assembly* assembly;
		SymbolTable* table;

		void ResolveInternal(SymbolRef* ref)
		{
			auto index = table->FindNodeIndexPart(ref->GetName());
			if (index == -1)
			{
				HXSL_ASSERT(false, "Couldn't find type.");
			}
			ref->SetTable(table, index);
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
			func->SetName(TextSpan(table->GetStringPool().add(name)));
			return *this;
		}

		FunctionBuilder& WithParam(const std::string& paramName, const std::string& paramType)
		{
			auto param = std::make_unique<Parameter>();
			param->SetName(TextSpan(table->GetStringPool().add(paramName)));

			auto paramRef = std::make_unique<SymbolRef>(TextSpan(table->GetStringPool().add(paramType)), SymbolRefType_AnyType);
			ResolveInternal(paramRef.get());
			param->SetSymbolRef(std::move(paramRef));

			parameters.push_back(std::move(param));
			return *this;
		}

		FunctionBuilder& Returns(const std::string& returnType)
		{
			auto returnRef = std::make_unique<SymbolRef>(TextSpan(table->GetStringPool().add(returnType)), SymbolRefType_AnyType);
			ResolveInternal(returnRef.get());
			func->SetReturnSymbol(std::move(returnRef));
			return *this;
		}

		void AttachToClass(Class* _class)
		{
			AttachToContainer(_class, _class->GetTableIndex());
		}

		void AttachToStruct(Struct* _struct)
		{
			AttachToContainer(_struct, _struct->GetTableIndex());
		}

		void AttachToContainer(Container* container, size_t index)
		{
			auto funcPtr = func.get();
			container->AddFunction(std::move(func));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				funcPtr->AddParameter(std::move(parameters[i]));
			}

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Function, SymbolScopeType_Class, AccessModifier_Public, 0, funcPtr);
			auto signature = funcPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(TextSpan(signature), meta, index);
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
			param->SetName(TextSpan(table->GetStringPool().add(paramName)));

			auto paramRef = std::make_unique<SymbolRef>(TextSpan(table->GetStringPool().add(paramType)), SymbolRefType_AnyType);
			param->SetSymbolRef(std::move(paramRef));

			parameters.push_back(std::move(param));
			return *this;
		}

		OperatorBuilder& Returns(const std::string& returnType)
		{
			auto returnRef = std::make_unique<SymbolRef>(TextSpan(table->GetStringPool().add(returnType)), SymbolRefType_AnyType);
			_operator->SetReturnSymbol(std::move(returnRef));
			return *this;
		}

		void AttachToClass(Class* _class)
		{
			AttachToContainer(_class, _class->GetTableIndex());
		}

		void AttachToStruct(Struct* _struct)
		{
			AttachToContainer(_struct, _struct->GetTableIndex());
		}

		void AttachToContainer(Container* container, size_t index)
		{
			auto operatorPtr = _operator.get();
			auto& op = _operator->GetOperator();
			auto& name = _operator->GetOperator() == Operator_Cast ? table->GetStringPool().add("operator") : table->GetStringPool().add("operator" + ToString(op));
			operatorPtr->SetName(TextSpan(name));
			container->AddOperator(std::move(_operator));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				operatorPtr->AddParameter(std::move(parameters[i]));
			}

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Operator, SymbolScopeType_Class, AccessModifier_Public, 0, operatorPtr);
			auto signature = operatorPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(TextSpan(signature), meta, index);
			operatorPtr->SetAssembly(assembly, funcIndex);
		}

		void AttachToPrimitive(Primitive* container)
		{
			auto operatorPtr = _operator.get();
			container->AddOperator(std::move(_operator));

			for (size_t i = 0; i < parameters.size(); i++)
			{
				ResolveInternal(parameters[i]->GetSymbolRef().get());
				operatorPtr->AddParameter(std::move(parameters[i]));
			}
			ResolveInternal(operatorPtr->GetReturnSymbolRef().get());
			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Operator, SymbolScopeType_Struct, AccessModifier_Public, 0, operatorPtr);
			auto signature = operatorPtr->BuildOverloadSignature();
			auto funcIndex = table->Insert(TextSpan(signature), meta, container->GetTableIndex());
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
			_class->SetName(TextSpan(table->GetStringPool().add(name)));
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
			param->SetName(TextSpan(table->GetStringPool().add(name)));

			auto paramRef = std::make_unique<SymbolRef>(TextSpan(table->GetStringPool().add(type)), SymbolRefType_AnyType);
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
	public:
		PrimitiveBuilder(Assembly* assembly) : ASTNodeBuilder(assembly), primitive(std::make_unique<Primitive>())
		{
		}

		PrimitiveBuilder& WithName(const std::string& name)
		{
			primitive->SetName(TextSpan(table->GetStringPool().add(name)));
			return *this;
		}

		PrimitiveBuilder& WithKind(PrimitiveKind kind, PrimitiveClass _class)
		{
			primitive->SetKind(kind);
			primitive->SetClass(_class);
			return *this;
		}

		PrimitiveBuilder& WithRowsAndColumns(uint rows, uint columns)
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

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& str)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(OperatorFlags_None, op)
					.WithParam("left", str)
					.WithParam("right", str)
					.Returns(str);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& strA, const std::string& strB, const std::string& strRet)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(OperatorFlags_None, op)
					.WithParam("left", strA)
					.WithParam("right", strB)
					.Returns(strRet);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str)
		{
			for (auto op : ops)
			{
				auto builder = std::make_unique<OperatorBuilder>(assembly);

				builder->WithOp(OperatorFlags_None, op)
					.WithParam("value", str)
					.Returns(str);

				auto ptr = builder.get();
				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str, const std::string& strRet)
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

		void WithImplicitCast(const std::string& str, const std::string& strRet)
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);

			builder->WithOp(OperatorFlags_Implicit, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			auto ptr = builder.get();
			operators.push_back(std::move(builder));
		}

		void WithExplicitCast(const std::string& str, const std::string& strRet)
		{
			auto builder = std::make_unique<OperatorBuilder>(assembly);

			builder->WithOp(OperatorFlags_Explicit, Operator_Cast)
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
	};

	static std::string toString(PrimitiveKind kind)
	{
		switch (kind)
		{
		case PrimitiveKind_Void:
			return "void";
			break;
		case PrimitiveKind_Bool:
			return "bool";
			break;
		case PrimitiveKind_Int:
			return "int";
			break;
		case PrimitiveKind_Float:
			return "float";
			break;
		case PrimitiveKind_Uint:
			return "uint";
			break;
		case PrimitiveKind_Double:
			return "double";
			break;
		case PrimitiveKind_Min8Float:
			return "min8float";
			break;
		case PrimitiveKind_Min10Float:
			return "min10float";
			break;
		case PrimitiveKind_Min16Float:
			return "min16float";
			break;
		case PrimitiveKind_Min12Int:
			return "min12int";
			break;
		case PrimitiveKind_Min16Int:
			return "min16int";
			break;
		case PrimitiveKind_Min16Uint:
			return "min16uint";
			break;
		default:
			return "";
			break;
		}
	}

	class PrimitiveManager
	{
	public:
		static PrimitiveManager& GetInstance()
		{
			static PrimitiveManager instance;

			std::call_once(initFlag, []() {
				instance.assembly = Assembly::Create("HXSL.Core");
				instance.Populate();
				instance.assembly->Seal();
				});

			return instance;
		}

		const SymbolTable* GetSymbolTable() const
		{
			return assembly->GetSymbolTable();
		}

		void Populate();

	private:
		static std::once_flag initFlag;

		SymbolTable* GetMutableSymbolTable() const
		{
			return assembly->GetMutableSymbolTable();
		}

		PrimitiveManager() = default;

		PrimitiveManager(const PrimitiveManager&) = delete;
		PrimitiveManager& operator=(const PrimitiveManager&) = delete;

		std::unique_ptr<Assembly> assembly;

		void AddPrimClass(TextSpan name, Class** outClass = nullptr, size_t* indexOut = nullptr);
		void AddPrim(PrimitiveKind kind, PrimitiveClass primitiveClass, uint rows, uint columns);
		void ResolveInternal(SymbolRef* ref);
	};

	class SwizzleManager
	{
	private:
		std::unique_ptr<SymbolTable> swizzleTable;
		std::vector<std::unique_ptr<SwizzleDefinition>> definitions;
		PrimitiveManager& primitives;

		static char NormalizeSwizzleChar(const char& c)
		{
			switch (c) {
			case 'r': return 'x';
			case 'g': return 'y';
			case 'b': return 'z';
			case 'a': return 'w';
			case 's': return 'x';
			case 't': return 'y';
			case 'p': return 'z';
			case 'q': return 'w';
			default: return c;
			}
		}
		static int SwizzleCharToIndex(char c)
		{
			switch (c) {
			case 'x': return 0;
			case 'y': return 1;
			case 'z': return 2;
			case 'w': return 3;
			default: return -1;
			}
		}

	public:
		SwizzleManager() : swizzleTable(std::make_unique<SymbolTable>()), primitives(PrimitiveManager::GetInstance())
		{
		}

		bool VerifySwizzle(Primitive* prim, SymbolRef* ref)
		{
			auto _class = prim->GetClass();
			if (_class == PrimitiveClass_Matrix) return false;
			auto& pattern = ref->GetName();
			if (pattern.Length < 1 || pattern.Length > 4)
				return false;

			auto index = swizzleTable->FindNodeIndexPart(pattern);

			if (index != -1)
			{
				ref->SetTable(swizzleTable.get(), index);
				return true;
			}

			auto componentCount = (int)prim->GetRows();

			for (auto& c : pattern)
			{
				char n = NormalizeSwizzleChar(c);
				int i = SwizzleCharToIndex(n);
				if (i < 0 || i >= componentCount)
				{
					return false;
				}
			}

			std::string typeName = toString(prim->GetKind()) + std::to_string(pattern.Length);

			auto primitivesTable = primitives.GetSymbolTable();
			auto primitiveIndex = primitivesTable->FindNodeIndexPart(typeName);
			auto& resultingType = primitivesTable->GetNode(primitiveIndex).Metadata->declaration;

			auto symbolRef = std::make_unique<SymbolRef>(Token(), SymbolRefType_Member);
			symbolRef->SetTable(primitivesTable, primitiveIndex);
			auto swizzleDef = std::make_unique<SwizzleDefinition>(resultingType->GetSpan(), std::move(symbolRef));
			auto metaField = std::make_shared<SymbolMetadata>(SymbolType_Field, SymbolScopeType_Struct, AccessModifier_Public, 0, swizzleDef.get());

			index = swizzleTable->Insert(pattern, metaField, 0);
			ref->SetTable(swizzleTable.get(), index);

			definitions.push_back(std::move(swizzleDef));

			return true;
		}
	};
}
#endif