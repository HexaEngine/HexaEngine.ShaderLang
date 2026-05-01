#ifndef MODULE_DECOMPILER_HPP
#define MODULE_DECOMPILER_HPP

#include "pch/ast_analyzers.hpp"
#include "core/module.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	class ModuleDecompiler
	{
		dense_map<Backend::Layout*, SymbolDef*> map;

		SymbolRef* MakeTypeRef(Backend::TypeLayout* type);

		void FillDataType(Backend::StructLayout* strct,
			std::vector<Field*>& fields,
			std::vector<Struct*>& structs,
			std::vector<Class*>& classes,
			std::vector<ConstructorOverload*>& ctors,
			std::vector<FunctionOverload*>& funcs,
			std::vector<OperatorOverload*>& ops);

	public:
		ModuleDecompiler() = default;

		SymbolDef* DeconvertType(Backend::TypeLayout* type);
		Primitive* DeconvertPrimitive(Backend::PrimitiveLayout* prim);
		Field* DeconvertField(Backend::FieldLayout* field);
		Parameter* DeconvertParameter(Backend::ParameterLayout* param);
		FunctionOverload* DeconvertFunction(Backend::FunctionLayout* func);
		OperatorOverload* DeconvertOperator(Backend::OperatorLayout* op);
		ConstructorOverload* DeconvertConstructor(Backend::ConstructorLayout* ctor, Backend::StructLayout* parent);
		Struct* DeconvertStruct(Backend::StructLayout* strct);
		Class* DeconvertClass(Backend::StructLayout* clss);
		Enum* DeconvertEnum(Backend::EnumLayout* enm);
		Namespace* DeconvertNamespace(Backend::NamespaceLayout* ns);

		[[nodiscard]] CompilationUnit* Deconvert(Backend::Module* module);
	};
}

#endif
