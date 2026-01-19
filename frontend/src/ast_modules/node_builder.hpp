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
		IdentifierInfo* name_m;
		SymbolRef* returnType_m;
		std::vector<Parameter*> parameters;
	public:
		FunctionBuilder(Assembly* assembly) : ASTNodeBuilder(assembly)
		{
		}

		FunctionBuilder& WithName(const std::string& name)
		{
			auto* context = ASTContext::GetCurrentContext();
			name_m = context->GetIdentifierTable().Get(name);
			return *this;
		}

		FunctionBuilder& WithParam(const std::string& paramName, const std::string& paramType, ParameterFlags flags = ParameterFlags_None, InterpolationModifier interpolMod = InterpolationModifier_Linear)
		{
			auto* context = ASTContext::GetCurrentContext();
			auto& idTable = context->GetIdentifierTable();
			auto paramRef = SymbolRef::Create({}, idTable.Get(paramType), SymbolRefType_Type, false);
			ResolveInternal(paramRef);

			auto param = Parameter::Create({}, idTable.Get(paramName), flags, interpolMod, paramRef, nullptr);
			parameters.push_back(param);
			return *this;
		}

		FunctionBuilder& Returns(const std::string& returnType)
		{
			auto* context = ASTContext::GetCurrentContext();
			returnType_m = SymbolRef::Create({}, context->GetIdentifierTable().Get(returnType), SymbolRefType_Type, false);
			ResolveInternal(returnType_m);
			return *this;
		}

		FunctionOverload* Finish(SymbolHandle root)
		{
			auto params = Span<Parameter*>(parameters);
			auto ptr = FunctionOverload::Create({}, name_m, AccessModifier_Public, FunctionFlags_None, returnType_m, nullptr, nullptr, params, {});

			for (auto& param : ptr->GetParameters())
			{
				ResolveInternal(param->GetSymbolRef());
			}
			ResolveInternal(returnType_m);

			auto meta = SymbolMetadata::Create(ptr);
			auto signature = ptr->BuildOverloadSignature();
			auto funcIndex = table->Insert(signature, meta, root);
			ptr->SetAssembly(assembly, funcIndex);

			return ptr;
		}
	};

	class OperatorBuilder : ASTNodeBuilder
	{
		OperatorFlags flags_m;
		Operator op_m;
		SymbolRef* returnType_m;
		std::vector<Parameter*> parameters;
	public:
		OperatorBuilder(Assembly* assembly) : ASTNodeBuilder(assembly)
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
			auto* context = ASTContext::GetCurrentContext();
			auto& idTable = context->GetIdentifierTable();
			auto paramRef = SymbolRef::Create({}, idTable.Get(paramType), SymbolRefType_Type, false);
	
			auto param = Parameter::Create({}, idTable.Get(paramName), flags, interpolMod, paramRef, nullptr);
			parameters.push_back(param);
			return *this;
		}

		OperatorBuilder& Returns(const std::string& returnType)
		{
			auto* context = ASTContext::GetCurrentContext();
			returnType_m = SymbolRef::Create({}, context->GetIdentifierTable().Get(returnType), SymbolRefType_Type, false);
			return *this;
		}

		OperatorOverload* Finish(const SymbolHandle& root)
		{
			auto* context = ASTContext::GetCurrentContext();
			auto name = context->GetIdentifierTable().Get(op_m == Operator_Cast ? "operator" : "operator" + ToString(op_m));
			auto params = Span<Parameter*>(parameters);
			auto ptr = OperatorOverload::Create({}, name, AccessModifier_Public, FunctionFlags_None, flags_m, op_m, returnType_m, nullptr, params, {});

			for (auto& param : ptr->GetParameters())
			{
				ResolveInternal(param->GetSymbolRef());
			}
			ResolveInternal(returnType_m);

			auto meta = SymbolMetadata::Create(ptr);
			auto signature = ptr->BuildOverloadSignature(false);
			auto funcIndex = table->Insert(signature, meta, root);
			ptr->SetAssembly(assembly, funcIndex);
			return ptr;
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
		FieldBuilder(Assembly* assembly) : ASTNodeBuilder(assembly)
		{
		}

		FieldBuilder& WithName(const std::string& name)
		{
			auto* context = ASTContext::GetCurrentContext();
			name_m = context->GetIdentifierTable().Get(name);
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
			auto* context = ASTContext::GetCurrentContext();
			type_m = SymbolRef::Create({}, context->GetIdentifierTable().Get(type), SymbolRefType_Type, false);
			return *this;
		}

		FieldBuilder& WithSemantic(const std::string& semantic)
		{
			auto* context = ASTContext::GetCurrentContext();
			semantic_m = context->GetIdentifierTable().Get(semantic);
			return *this;
		}

		Field* Finish(const SymbolHandle& root)
		{
			auto field = Field::Create({}, name_m, access_m, storageClass_m, interpolMod_m, type_m, semantic_m);
			ResolveInternal(type_m);

			auto meta = SymbolMetadata::Create(field);
			auto fieldIndex = table->Insert(field->GetName(), meta, root);
			field->SetAssembly(assembly, fieldIndex);
			return field;
		}
	};

	class ClassBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m = nullptr;
		std::vector<FieldBuilder> fields;
		std::vector<FunctionBuilder> functions;
		std::vector<OperatorBuilder> operators;
	public:
		ClassBuilder(Assembly* assembly) : ASTNodeBuilder(assembly)
		{
		}

		ClassBuilder& WithName(const std::string& name)
		{
			auto* context = ASTContext::GetCurrentContext();
			name_m = context->GetIdentifierTable().Get(name);
			return *this;
		}

		FunctionBuilder& WithFunction()
		{
			auto idx = functions.size();
			functions.push_back(FunctionBuilder(assembly));
			return functions[idx];
		}

		FieldBuilder& WithField()
		{
			auto idx = fields.size();
			fields.push_back(FieldBuilder(assembly));
			return fields[idx];
		}

		OperatorBuilder& WithOperator()
		{
			auto idx = operators.size();
			operators.push_back(OperatorBuilder(assembly));
			return operators[idx];
		}

		Class* Finish()
		{
			auto pClass = Class::Create({}, name_m, AccessModifier_Public, static_cast<uint32_t>(fields.size()), 0, 0, 0, static_cast<uint32_t>(functions.size()), static_cast<uint32_t>(operators.size()));
			auto meta = SymbolMetadata::Create(pClass);
			auto index = table->Insert(pClass->GetName(), meta);
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

			return pClass;
		}
	};

	class PrimitiveBuilder : ASTNodeBuilder
	{
		IdentifierInfo* name_m = nullptr;
		PrimitiveKind kind_m = PrimitiveKind_Void;
		PrimitiveClass _class_m = PrimitiveClass_Scalar;
		uint32_t rows_m = 0;
		uint32_t columns_m = 0;

		Primitive* prim = nullptr;
		std::vector<OperatorBuilder> operators;
	public:
		PrimitiveBuilder(Assembly* assembly) : ASTNodeBuilder(assembly)
		{
		}

		PrimitiveBuilder& WithName(const StringSpan& name)
		{
			auto* context = ASTContext::GetCurrentContext();
			name_m = context->GetIdentifierTable().Get(name);
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
			operators.push_back(OperatorBuilder(assembly));
			return operators[idx];
		}

		void WithBinaryOperators(std::initializer_list<Operator> ops, const std::string& str, OperatorFlags flags)
		{
			for (auto op : ops)
			{
				auto builder = OperatorBuilder(assembly);

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
				auto builder = OperatorBuilder(assembly);

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
				auto builder = OperatorBuilder(assembly);

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
				auto builder = OperatorBuilder(assembly);

				builder.WithOp(OperatorFlags_None, op)
					.WithParam("value", str)
					.Returns(strRet);

				operators.push_back(std::move(builder));
			}
		}

		void WithImplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = OperatorBuilder(assembly);

			builder.WithOp(OperatorFlags_Implicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			operators.push_back(std::move(builder));
		}

		void WithExplicitCast(const std::string& str, const std::string& strRet, OperatorFlags flags)
		{
			auto builder = OperatorBuilder(assembly);

			builder.WithOp(OperatorFlags_Explicit | flags, Operator_Cast)
				.WithParam("value", str)
				.Returns(strRet);

			operators.push_back(std::move(builder));
		}

	private:
		void MakePrimitive()
		{
			prim = Primitive::Create({}, name_m, kind_m, _class_m, rows_m, columns_m, static_cast<uint32_t>(operators.size()));

			auto meta = SymbolMetadata::Create(prim);
			auto index = table->Insert(prim->GetName(), meta);
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
					operatorsDst[i]->SetParent(prim);
				}
				operators.clear();
				return true;
			}
		}
	};
}

#endif