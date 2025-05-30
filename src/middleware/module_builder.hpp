#ifndef AST_TO_MODULE_CONVERTER_HPP
#define AST_TO_MODULE_CONVERTER_HPP

#include "pch/ast_analyzers.hpp"
#include "core/module.hpp"
#include "core/layout_builder.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	class ModuleBuilder
	{
		std::unique_ptr<Backend::Module> module;
		dense_map<Backend::TypeLayout*, Backend::PointerLayout*> pointerMap;
		dense_map<SymbolDef*, Backend::Layout*> map;
		std::vector<Backend::FunctionLayout*> functions;

		void ConvertFunction(FunctionOverload* func, Backend::FunctionLayoutBuilder& builder);

		void ConvertType(Type* type, Backend::StructLayoutBuilder& builder);

	public:
		Backend::PointerLayout* MakePointerType(Backend::TypeLayout* elementType);

		Backend::PointerLayout* MakePointerType(SymbolDef* elementType);

		Backend::TypeLayout* ConvertType(SymbolDef* type);

		Backend::PrimitiveLayout* ConvertPrimitive(Primitive* prim);

		Backend::FieldLayout* ConvertField(Field* field);

		Backend::ParameterLayout* ConvertParameter(Parameter* param);

		Backend::FunctionLayout* ConvertFunction(FunctionOverload* func);

		Backend::OperatorLayout* ConvertOperator(OperatorOverload* op);

		Backend::ConstructorLayout* ConvertConstructor(ConstructorOverload* ctor);

		Backend::StructLayout* ConvertStruct(Struct* strct);

		Backend::StructLayout* ConvertClass(Class* clss);

		Backend::NamespaceLayout* ConvertNamespace(Namespace* ns);

		Backend::Module* GetModule() const { return module.get(); }

		[[nodiscard]] std::unique_ptr<Backend::Module> Convert(CompilationUnit* compilation);
	};
}

#endif