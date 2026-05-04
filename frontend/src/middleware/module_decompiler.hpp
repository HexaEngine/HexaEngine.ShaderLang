#ifndef MODULE_DECOMPILER_HPP
#define MODULE_DECOMPILER_HPP

#include "pch/ast_analyzers.hpp"
#include "core/module.hpp"
#include "utils/dense_map.hpp"
#include "module_stub.hpp"

namespace HXSL
{
	struct ModuleDecompilerOptions
	{
		bool markAsExtern = false;
	};

	class ModuleDecompiler
	{
		ModuleDecompilerOptions options;
		dense_map<Backend::Layout*, SymbolDef*> map;
		dense_map<ASTNode*, Backend::Layout*> reverseMap;

		SymbolRef* MakeTypeRef(Backend::TypeLayout* type);

		void FillDataType(Backend::StructLayout* strct,
			std::vector<Field*>& fields,
			std::vector<Struct*>& structs,
			std::vector<Class*>& classes,
			std::vector<ConstructorOverload*>& ctors,
			std::vector<FunctionOverload*>& funcs,
			std::vector<OperatorOverload*>& ops);
		
		void Transform(Backend::Layout* layout, ASTNode* node);

	public:
		ModuleDecompiler(ModuleDecompilerOptions& options) : options(options)
		{
		}

		dense_map<ASTNode*, Backend::Layout*>& GetReverseMap() { return reverseMap; }

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

		[[nodiscard]] uptr<StubModule> Deconvert(Backend::Module* module);
	};
}

#endif
