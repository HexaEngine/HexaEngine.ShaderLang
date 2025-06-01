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
		ASTContext* context;
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

		ASTNodeBuilder(ASTContext* context, Assembly* assembly) : context(context), assembly(assembly), table(assembly->GetMutableSymbolTable())
		{
		}
	};

	class FunctionBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m;
		SymbolRef* returnType_m;
		std::vector<ast_ptr<Parameter>> parameters;
	public:
		FunctionBuilder(ASTContext* context, Assembly* assembly) : ASTNodeBuilder(context, assembly)
		{
		}

		FunctionBuilder& WithName(const std::string& name)
		{
			name_m = context->GetIdentiferTable().Get(name);
			return *this;
		}

		FunctionBuilder& WithParam(const std::string& paramName, const std::string& paramType, ParameterFlags flags = ParameterFlags_None, InterpolationModifier interpolMod = InterpolationModifier_Linear)
		{
			auto& idTable = context->GetIdentiferTable();
			auto paramRef = ast_ptr<SymbolRef>(SymbolRef::Create(context, {}, idTable.Get(paramType), SymbolRefType_Type, false));
			ResolveInternal(paramRef.get());

			auto param = Parameter::Create(context, {}, idTable.Get(paramName), flags, interpolMod, std::move(paramRef), nullptr);
			parameters.push_back(ast_ptr<Parameter>(param));
			return *this;
		}

		FunctionBuilder& Returns(const std::string& returnType)
		{
			returnType_m = SymbolRef::Create(context, {}, context->GetIdentiferTable().Get(returnType), SymbolRefType_Type, false);
			ResolveInternal(returnType_m);
			return *this;
		}

		ast_ptr<FunctionOverload> Finish(SymbolHandle root)
		{
			auto params = ArrayRef<ast_ptr<Parameter>>(parameters);
			auto ptr = FunctionOverload::Create(context, {}, name_m, AccessModifier_Public, FunctionFlags_None, ast_ptr<SymbolRef>(returnType_m), params);

			for (auto& param : ptr->GetParameters())
			{
				ResolveInternal(param->GetSymbolRef().get());
			}
			ResolveInternal(returnType_m);

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Function, SymbolScopeType_Class, AccessModifier_Public, 0, ptr);
			auto signature = ptr->BuildOverloadSignature();
			auto funcIndex = table->Insert(signature, meta, root.GetIndex());
			ptr->SetAssembly(assembly, funcIndex);

			return ast_ptr<FunctionOverload>(ptr);
		}
	};

	class OperatorBuilder : ASTNodeBuilder
	{
		OperatorFlags flags_m;
		Operator op_m;
		SymbolRef* returnType_m;
		std::vector<ast_ptr<Parameter>> parameters;
	public:
		OperatorBuilder(ASTContext* context, Assembly* assembly) : ASTNodeBuilder(context, assembly)
		{
		}

		OperatorBuilder& WithOp(const OperatorFlags& flags, const Operator& op)
		{
			flags_m = flags;
			op_m = op;
			return *this;
		}

		OperatorBuilder& WithParam(const std::string& paramName, const std::string& paramType, ParameterFlags flags = ParameterFlags_None, InterpolationModifier interpolMod = InterpolationModifier_Linear)
		{
			auto& idTable = context->GetIdentiferTable();
			auto paramRef = ast_ptr<SymbolRef>(SymbolRef::Create(context, {}, idTable.Get(paramType), SymbolRefType_Type, false));
			ResolveInternal(paramRef.get());

			auto param = Parameter::Create(context, {}, idTable.Get(paramName), flags, interpolMod, std::move(paramRef), nullptr);
			parameters.push_back(ast_ptr<Parameter>(param));
			return *this;
		}

		OperatorBuilder& Returns(const std::string& returnType)
		{
			returnType_m = SymbolRef::Create(context, {}, context->GetIdentiferTable().Get(returnType), SymbolRefType_Type, false);
			return *this;
		}

		ast_ptr<OperatorOverload> Finish(const SymbolHandle& root)
		{
			auto name = context->GetIdentiferTable().Get(op_m == Operator_Cast ? "operator" : "operator" + ToString(op_m));
			auto params = ArrayRef<ast_ptr<Parameter>>(parameters);
			auto ptr = OperatorOverload::Create(context, {}, name, AccessModifier_Public, FunctionFlags_None, flags_m, op_m, ast_ptr<SymbolRef>(returnType_m), params);

			for (auto& param : ptr->GetParameters())
			{
				ResolveInternal(param->GetSymbolRef().get());
			}
			ResolveInternal(returnType_m);

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Operator, SymbolScopeType_Struct, AccessModifier_Public, 0, ptr);
			auto signature = ptr->BuildOverloadSignature(false);
			auto funcIndex = table->Insert(signature, meta, root.GetIndex());
			ptr->SetAssembly(assembly, funcIndex);
			return ast_ptr<OperatorOverload>(ptr);
		}
	};

	class FieldBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m = nullptr;
		AccessModifier access_m = AccessModifier_Private;
		StorageClass storageClass_m = StorageClass_None;
		InterpolationModifier interpolMod_m = InterpolationModifier_Linear;
		SymbolRef* type_m = nullptr;
		IdentifierInfo* semantic_m = nullptr;

	public:
		FieldBuilder(ASTContext* context, Assembly* assembly) : ASTNodeBuilder(context, assembly)
		{
		}

		FieldBuilder& WithName(const std::string& name)
		{
			name_m = context->GetIdentiferTable().Get(name);
			return *this;
		}

		FieldBuilder& WithAccessModifier(AccessModifier access)
		{
			access_m = access;
			return *this;
		}

		FieldBuilder& WithStorageClass(StorageClass storageClass)
		{
			storageClass_m = storageClass;
			return *this;
		}

		FieldBuilder& WithInterpolationModifier(InterpolationModifier interpolMod)
		{
			interpolMod_m = interpolMod;
			return *this;
		}

		FieldBuilder& WithType(const std::string& type)
		{
			type_m = SymbolRef::Create(context, {}, context->GetIdentiferTable().Get(type), SymbolRefType_Type, false);
			return *this;
		}

		FieldBuilder& WithSemantic(const std::string& semantic)
		{
			semantic_m = context->GetIdentiferTable().Get(semantic);
			return *this;
		}

		ast_ptr<Field> Finish(const SymbolHandle& root)
		{
			auto field = Field::Create(context, {}, name_m, access_m, storageClass_m, interpolMod_m, ast_ptr<SymbolRef>(type_m), semantic_m);
			ResolveInternal(type_m);

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Field, SymbolScopeType_Struct, access_m, 0, field);
			auto fieldIndex = table->Insert(field->GetName(), meta, root.GetIndex());
			field->SetAssembly(assembly, fieldIndex);
			return ast_ptr<Field>(field);
		}
	};

	class ClassBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m = nullptr;
		std::vector<FieldBuilder> fields;
		std::vector<FunctionBuilder> functions;
		std::vector<OperatorBuilder> operators;
	public:
		ClassBuilder(ASTContext* context, Assembly* assembly) : ASTNodeBuilder(context, assembly)
		{
		}

		ClassBuilder& WithName(const std::string& name)
		{
			name_m = context->GetIdentiferTable().Get(name);
			return *this;
		}

		FunctionBuilder& WithFunction()
		{
			auto idx = functions.size();
			functions.push_back(FunctionBuilder(context, assembly));
			return functions[idx];
		}

		FieldBuilder& WithField()
		{
			auto idx = fields.size();
			fields.push_back(FieldBuilder(context, assembly));
			return fields[idx];
		}

		OperatorBuilder& WithOperator()
		{
			auto idx = operators.size();
			operators.push_back(OperatorBuilder(context, assembly));
			return operators[idx];
		}

		ast_ptr<Class> Finish()
		{
			auto pClass = Class::Create(context, {}, name_m, AccessModifier_Public, static_cast<uint32_t>(fields.size()), 0, 0, 0, static_cast<uint32_t>(functions.size()), static_cast<uint32_t>(operators.size()));
			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Primitive, SymbolScopeType_Global, AccessModifier_Public, 0, pClass);
			auto index = table->Insert(pClass->GetName(), meta, 0);
			pClass->SetAssembly(assembly, index);

			auto fieldsDst = pClass->GetFields();
			for (size_t i = 0; i < fields.size(); ++i)
			{
				fieldsDst[i] = fields[i].Finish(index);
			}

			auto functionsDst = pClass->GetFunctions();
			for (size_t i = 0; i < fields.size(); ++i)
			{
				functionsDst[i] = functions[i].Finish(index);
			}

			auto operatorsDst = pClass->GetFunctions();
			for (size_t i = 0; i < fields.size(); ++i)
			{
				operatorsDst[i] = operators[i].Finish(index);
			}

			name_m = nullptr;
			fields.clear();
			functions.clear();
			operators.clear();

			return ast_ptr<Class>(pClass);
		}
	};

	class PrimitiveBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m;
		PrimitiveKind kind_m;
		PrimitiveClass _class_m;
		uint32_t rows_m;
		uint32_t columns_m;

		Primitive* prim;
		std::vector<OperatorBuilder> operators;
	public:
		PrimitiveBuilder(ASTContext* context, Assembly* assembly) : ASTNodeBuilder(context, assembly)
		{
		}

		PrimitiveBuilder& WithName(const StringSpan& name)
		{
			name_m = context->GetIdentiferTable().Get(name);
			return *this;
		}

		PrimitiveBuilder& WithKind(PrimitiveKind kind, PrimitiveClass _class)
		{
			kind_m = kind;
			_class_m = _class;
			return *this;
		}

		PrimitiveBuilder& WithRowsAndColumns(uint32_t rows, uint32_t columns)
		{
			rows_m = rows;
			columns_m = columns;
			return *this;
		}

		OperatorBuilder& WithOperator()
		{
			auto idx = operators.size();
			operators.push_back(OperatorBuilder(context, assembly));
			return operators[idx];
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& str, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = OperatorBuilder(context, assembly);

				builder.WithOp(flags, op)
					.WithParam("left", str)
					.WithParam("right", str)
					.Returns(str);

				operators.push_back(std::move(builder));
			}
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& strA, const std::string& strB, const std::string& strRet, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = OperatorBuilder(context, assembly);

				builder.WithOp(flags, op)
					.WithParam("left", strA)
					.WithParam("right", strB)
					.Returns(strRet);

				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = OperatorBuilder(context, assembly);

				builder.WithOp(flags, op)
					.WithParam("value", str)
					.Returns(str);

				operators.push_back(std::move(builder));
			}
		}

		void WithUnaryOperators(std::initializer_list<Operator> ops, const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = OperatorBuilder(context, assembly);

				builder.WithOp(OperatorFlags_None, op)
					.WithParam("value", str)
					.Returns(strRet);

				operators.push_back(std::move(builder));
			}
		}

		void WithImplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = OperatorBuilder(context, assembly);

			builder.WithOp(OperatorFlags_Implicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			operators.push_back(std::move(builder));
		}

		void WithExplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = OperatorBuilder(context, assembly);

			builder.WithOp(OperatorFlags_Explicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			operators.push_back(std::move(builder));
		}

	private:
		void MakePrimitive()
		{
			auto compilation = assembly->GetMutableSymbolTable()->GetCompilation();
			prim = Primitive::Create(context, {}, name_m, kind_m, _class_m, rows_m, columns_m, static_cast<uint32_t>(operators.size()));

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Primitive, SymbolScopeType_Global, AccessModifier_Public, 0, prim);
			auto index = table->Insert(prim->GetName(), meta, 0);
			prim->SetAssembly(assembly, index);
		}

	public:
		void Finish()
		{
			MakePrimitive();

			auto& index = prim->GetSymbolHandle();
			auto operatorsDst = prim->GetOperators();
			for (size_t i = 0; i < operators.size(); i++)
			{
				operatorsDst[i] = operators[i].Finish(index);
			}
		}

		bool FinishPhased()
		{
			if (prim == nullptr)
			{
				MakePrimitive();
				return false;
			}
			else
			{
				auto& index = prim->GetSymbolHandle();
				auto operatorsDst = prim->GetOperators();
				for (size_t i = 0; i < operators.size(); i++)
				{
					operatorsDst[i] = operators[i].Finish(index);
				}
				operators.clear();
				return true;
			}
		}
	};
}

#endif