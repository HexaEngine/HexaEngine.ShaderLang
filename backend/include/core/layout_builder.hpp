#ifndef LAYOUT_BUILDER_HPP
#define LAYOUT_BUILDER_HPP

#include "module.hpp"
#include "lexical/identifier_table.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ParameterLayoutBuilder;
		class FunctionLayoutBuilder;
		class OperatorLayoutBuilder;
		class FieldLayoutBuilder;
		class StructLayoutBuilder;
		class PrimitiveLayoutBuilder;

		class LayoutBuilderBase
		{
		protected:
			BumpAllocator& allocator;

			LayoutBuilderBase(Module& module) : allocator(module.GetAllocator()) {}
		public:
			BumpAllocator& GetAllocator() { return allocator; }
		};

		class ParameterLayoutBuilder : public LayoutBuilderBase
		{
		private:
			ParameterLayout* param;

		public:
			explicit ParameterLayoutBuilder(Module& m) : LayoutBuilderBase(m), param(allocator.Alloc<ParameterLayout>())
			{
			}

			ParameterLayoutBuilder& Name(const StringSpan& name)
			{
				param->SetName(allocator.CopyString(name));
				return *this;
			}

			ParameterLayoutBuilder& Semantic(const IdentifierInfo* semantic)
			{
				StringSpan span = semantic ? semantic->name : StringSpan();
				param->SetSemantic(allocator.CopyString(span));
				return *this;
			}

			ParameterLayoutBuilder& Semantic(const StringSpan& semantic)
			{
				param->SetSemantic(allocator.CopyString(semantic));
				return *this;
			}

			ParameterLayoutBuilder& Type(TypeLayout* type)
			{
				param->SetType(type);
				return *this;
			}
			ParameterLayoutBuilder& StorageClass(StorageClass sc)
			{
				param->SetStorageClass(sc);
				return *this;
			}
			ParameterLayoutBuilder& InterpolationModifier(InterpolationModifier im)
			{
				param->SetInterpolationModifier(im);
				return *this;
			}
			ParameterLayoutBuilder& ParameterFlags(ParameterFlags pf)
			{
				param->SetParameterFlags(pf);
				return *this;
			}

			[[nodiscard]] ParameterLayout* Build()
			{
				return param;
			}
		};

		class FunctionLayoutBuilder : public LayoutBuilderBase
		{
		protected:
			FunctionLayout* func;
			std::vector<ParameterLayout*> parameters;

			explicit FunctionLayoutBuilder(Module& m, FunctionLayout* func) : LayoutBuilderBase(m), func(func)
			{
			}

		public:
			explicit FunctionLayoutBuilder(Module& m) : LayoutBuilderBase(m), func(allocator.Alloc<FunctionLayout>())
			{
			}

			FunctionLayout* Peek() { return func; }

			FunctionLayoutBuilder& Name(const StringSpan& name)
			{
				func->SetName(allocator.CopyString(name));
				return *this;
			}

			FunctionLayoutBuilder& ReturnType(TypeLayout* rt)
			{
				func->SetReturnType(rt);
				return *this;
			}

			FunctionLayoutBuilder& Access(AccessModifier access)
			{
				func->SetAccess(access);
				return *this;
			}

			FunctionLayoutBuilder& StorageClass(StorageClass sc)
			{
				func->SetStorageClass(sc);
				return *this;
			}

			FunctionLayoutBuilder& FunctionFlags(FunctionFlags ff)
			{
				func->SetFunctionFlags(ff);
				return *this;
			}

			FunctionLayoutBuilder& AddParameter(ParameterLayout* param)
			{
				param->SetParent(func);
				parameters.push_back(param);
				return *this;
			}

			FunctionLayoutBuilder& Context(ILContext* context)
			{
				func->SetContext(context);
				return *this;
			}

			FunctionLayoutBuilder& CodeBlob(ILCodeBlob* blob)
			{
				func->SetCodeBlob(blob);
				return *this;
			}

			[[nodiscard]] FunctionLayout* Build()
			{
				func->SetParameters(allocator.CopySpan(parameters));
				return func;
			}
		};

		class OperatorLayoutBuilder : public FunctionLayoutBuilder
		{
		private:
			OperatorLayout* op;

		public:
			explicit OperatorLayoutBuilder(Module& m) : FunctionLayoutBuilder(m, allocator.Alloc<OperatorLayout>())
			{
				op = static_cast<OperatorLayout*>(func);
			}

			OperatorLayoutBuilder& Operator(Operator o)
			{
				op->SetOperator(o);
				return *this;
			}

			OperatorLayoutBuilder& OperatorFlags(OperatorFlags of)
			{
				op->SetOperatorFlags(of);
				return *this;
			}

			[[nodiscard]] OperatorLayout* Build()
			{
				return op;
			}
		};

		class ConstructorLayoutBuilder : public FunctionLayoutBuilder
		{
		private:
			ConstructorLayout* ctor;

		public:
			explicit ConstructorLayoutBuilder(Module& m) : FunctionLayoutBuilder(m, allocator.Alloc<ConstructorLayout>())
			{
				ctor = static_cast<ConstructorLayout*>(func);
			}

			[[nodiscard]] ConstructorLayout* Build()
			{
				return ctor;
			}
		};

		class FieldLayoutBuilder : public LayoutBuilderBase
		{
		private:
			FieldLayout* field;

		public:
			explicit FieldLayoutBuilder(Module& m) : LayoutBuilderBase(m), field(allocator.Alloc<FieldLayout>())
			{
			}

			FieldLayoutBuilder& Name(const StringSpan& name)
			{
				field->SetName(allocator.CopyString(name));
				return *this;
			}

			FieldLayoutBuilder& Semantic(const IdentifierInfo* semantic)
			{
				StringSpan span = semantic ? semantic->name : StringSpan();
				field->SetSemantic(allocator.CopyString(span));
				return *this;
			}

			FieldLayoutBuilder& Semantic(const StringSpan& semantic)
			{
				field->SetSemantic(allocator.CopyString(semantic));
				return *this;
			}

			FieldLayoutBuilder& Type(TypeLayout* type)
			{
				field->SetType(type);
				return *this;
			}

			FieldLayoutBuilder& Access(AccessModifier access)
			{
				field->SetAccess(access);
				return *this;
			}

			FieldLayoutBuilder& StorageClass(StorageClass sc)
			{
				field->SetStorageClass(sc);
				return *this;
			}

			FieldLayoutBuilder& InterpolationModifier(InterpolationModifier im)
			{
				field->SetInterpolationModifier(im);
				return *this;
			}

			[[nodiscard]] FieldLayout* Build()
			{
				return field;
			}
		};

		class StructLayoutBuilder : public LayoutBuilderBase
		{
		private:
			StructLayout* strct;
			std::vector<FieldLayout*> fieldsVec;
			std::vector<FunctionLayout*> functionsVec;
			std::vector<OperatorLayout*> operatorsVec;
			std::vector<ConstructorLayout*> constructorVec;
			std::vector<StructLayout*> structVec;

		public:
			explicit StructLayoutBuilder(Module& m) : LayoutBuilderBase(m), strct(allocator.Alloc<StructLayout>())
			{
			}

			StructLayoutBuilder& Name(const StringSpan& name)
			{
				strct->SetName(allocator.CopyString(name));
				return *this;
			}

			StructLayoutBuilder& Access(AccessModifier access)
			{
				strct->SetAccess(access);
				return *this;
			}

			StructLayoutBuilder& StructFlags(StructLayoutFlags flags)
			{
				strct->SetStructFlags(flags);
				return *this;
			}

			StructLayoutBuilder& AddField(FieldLayout* field)
			{
				field->SetParent(strct);
				fieldsVec.push_back(field);
				return *this;
			}

			StructLayoutBuilder& AddFunction(FunctionLayout* func)
			{
				func->SetParent(strct);
				functionsVec.push_back(func);
				return *this;
			}

			StructLayoutBuilder& AddOperator(OperatorLayout* op)
			{
				op->SetParent(strct);
				operatorsVec.push_back(op);
				return *this;
			}

			StructLayoutBuilder& AddConstructor(ConstructorLayout* ctor)
			{
				ctor->SetParent(strct);
				constructorVec.push_back(ctor);
				return *this;
			}

			StructLayoutBuilder& AddType(StructLayout* type)
			{
				type->SetParent(strct);
				structVec.push_back(type);
				return *this;
			}

			[[nodiscard]] StructLayout* Build()
			{
				strct->SetFields(allocator.CopySpan(fieldsVec));
				strct->SetFunctions(allocator.CopySpan(functionsVec));
				strct->SetOperators(allocator.CopySpan(operatorsVec));
				strct->SetConstructors(allocator.CopySpan(constructorVec));
				strct->SetStructs(allocator.CopySpan(structVec));
				return strct;
			}
		};

		class PrimitiveLayoutBuilder : LayoutBuilderBase
		{
		private:
			PrimitiveLayout* prim;

		public:
			explicit PrimitiveLayoutBuilder(Module& m) : LayoutBuilderBase(m), prim(allocator.Alloc<PrimitiveLayout>())
			{
			}

			PrimitiveLayoutBuilder& Name(const StringSpan& name)
			{
				prim->SetName(allocator.CopyString(name));
				return *this;
			}

			PrimitiveLayoutBuilder& Access(AccessModifier access)
			{
				prim->SetAccess(access);
				return *this;
			}

			PrimitiveLayoutBuilder& Kind(PrimitiveKind kind)
			{
				prim->SetKind(kind);
				return *this;
			}

			PrimitiveLayoutBuilder& Class(PrimitiveClass klass)
			{
				prim->SetClass(klass);
				return *this;
			}

			PrimitiveLayoutBuilder& Rows(uint32_t rows)
			{
				prim->SetRows(rows);
				return *this;
			}

			PrimitiveLayoutBuilder& Columns(uint32_t cols)
			{
				prim->SetColumns(cols);
				return *this;
			}

			[[nodiscard]] PrimitiveLayout* Build()
			{
				return prim;
			}
		};

		class PointerLayoutBuilder : LayoutBuilderBase
		{
		private:
			PointerLayout* pointer;
		public:
			PointerLayoutBuilder(Module& module) : LayoutBuilderBase(module), pointer(allocator.Alloc<PointerLayout>())
			{
			}

			PointerLayoutBuilder& Name(const StringSpan& name)
			{
				pointer->SetName(allocator.CopyString(name));
				return *this;
			}

			PointerLayoutBuilder& Access(AccessModifier access)
			{
				pointer->SetAccess(access);
				return *this;
			}

			PointerLayoutBuilder& ElementType(TypeLayout* elementType)
			{
				pointer->SetElementType(elementType);
				return *this;
			}

			[[nodiscard]] PointerLayout* Build()
			{
				return pointer;
			}
		};

		class NamespaceLayoutBuilder : LayoutBuilderBase
		{
		private:
			NamespaceLayout* ns;
			std::vector<StructLayout*> structVec;
			std::vector<FieldLayout*> globalFieldsVec;
			std::vector<FunctionLayout*> functionsVec;
			std::vector<NamespaceLayout*> nestedNamespaceVec;

		public:
			NamespaceLayoutBuilder(Module& module) : LayoutBuilderBase(module), ns(allocator.Alloc<NamespaceLayout>())
			{
			}

			NamespaceLayoutBuilder& Name(const StringSpan& name)
			{
				ns->SetName(allocator.CopyString(name));
				return *this;
			}

			NamespaceLayoutBuilder& AddStruct(StructLayout* strct)
			{
				strct->SetParent(ns);
				structVec.push_back(strct);
				return *this;
			}

			NamespaceLayoutBuilder& AddFunction(FunctionLayout* func)
			{
				func->SetParent(ns);
				functionsVec.push_back(func);
				return *this;
			}

			NamespaceLayoutBuilder& AddGlobalField(FieldLayout* field)
			{
				field->SetParent(ns);
				globalFieldsVec.push_back(field);
				return *this;
			}

			NamespaceLayoutBuilder& AddNestedNamespace(NamespaceLayout* nestedNs)
			{
				nestedNs->SetParent(ns);
				nestedNamespaceVec.push_back(nestedNs);
				return *this;
			}

			[[nodiscard]] NamespaceLayout* Build()
			{
				ns->SetStructs(allocator.CopySpan(structVec));
				ns->SetGlobalFields(allocator.CopySpan(globalFieldsVec));
				ns->SetFunctions(allocator.CopySpan(functionsVec));
				ns->SetNestedNamespaces(allocator.CopySpan(nestedNamespaceVec));
				return ns;
			}
		};
	}
}

#endif