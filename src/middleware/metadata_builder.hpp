#ifndef METADATA_BUILDER_HPP
#define METADATA_BUILDER_HPP

#include "il/il_metadata.hpp"
#include "module_builder.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	class ILMetadataBuilder
	{
	protected:
		ModuleBuilder& builder;
		Backend::ILMetadata& metadata;
		dense_map<SymbolDef*, Backend::ILVarId> defToVar;
	public:
		ILMetadataBuilder(ModuleBuilder& builder, Backend::ILMetadata& metadata) : builder(builder), metadata(metadata) {}

		Backend::ILType RegType(SymbolDef* def)
		{
			return metadata.RegType(builder.ConvertType(def));
		}

		Backend::ILType RegType(Backend::TypeLayout* def)
		{
			return metadata.RegType(def);
		}

		Backend::ILVariable& RegVar(Backend::ILType typeId, SymbolDef* def)
		{
			auto it = defToVar.find(def);
			if (it != defToVar.end())
			{
				return metadata.variables[it->second];
			}
			auto& var = metadata.RegVar(typeId);
			defToVar.insert({ def, var.id });
			return var;
		}

		Backend::ILVariable& RegVar(SymbolDef* type, SymbolDef* var)
		{
			return RegVar(RegType(type), var);
		}

		Backend::ILFuncCall RegFunc(SymbolDef* def)
		{
			return metadata.RegFunc(builder.ConvertFunction(dynamic_cast<FunctionOverload*>(def)));
		}

		Backend::ILVariable& FindVar(SymbolDef* var)
		{
			auto it = defToVar.find(var);
			if (it != defToVar.end())
			{
				return metadata.variables[it->second.id()];
			}
			return metadata.invalid;
		}

		Backend::ILVariable& FindVar(MemberReferenceExpression* expr)
		{
			return FindVar(expr->GetSymbolRef()->GetDeclaration());
		}

		Backend::ILVariable& FindVar(MemberAccessExpression* expr)
		{
			return FindVar(expr->GetSymbolRef()->GetDeclaration());
		}

		Backend::ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef)
		{
			return metadata.MakeFieldAccess(builder.ConvertField(dynamic_cast<Field*>(fieldDef)));
		}

		Backend::ILVariable& MakeTemp(SymbolDef* type)
		{
			return metadata.RegTempVar(RegType(type));
		}

		Backend::ILVariable& MakeTemp(Backend::TypeLayout* type)
		{
			return metadata.RegTempVar(type);
		}

		Backend::PointerLayout* MakeAddrType(SymbolDef* elementType)
		{
			return builder.MakePointerType(elementType);
		}
	};

	class ILMetadataAdapter
	{
	protected:
		ILMetadataBuilder& metaBuilder;
	public:
		ILMetadataAdapter(ILMetadataBuilder& metaBuilder) : metaBuilder(metaBuilder) {}

		Backend::ILType RegType(SymbolDef* def) { return metaBuilder.RegType(def); }

		Backend::ILType RegType(Backend::TypeLayout* def) { return metaBuilder.RegType(def); }

		Backend::ILVariable& RegVar(Backend::ILType typeId, SymbolDef* def) { return metaBuilder.RegVar(typeId, def); }

		Backend::ILVariable& RegVar(SymbolDef* type, SymbolDef* var) { return metaBuilder.RegVar(type, var); }

		Backend::ILFuncCall RegFunc(SymbolDef* def) { return metaBuilder.RegFunc(def); }

		Backend::ILVariable& FindVar(SymbolDef* var) { return metaBuilder.FindVar(var); }

		Backend::ILVariable& FindVar(MemberReferenceExpression* expr) { return metaBuilder.FindVar(expr); }

		Backend::ILVariable& FindVar(MemberAccessExpression* expr) { return metaBuilder.FindVar(expr); }

		Backend::ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef) { return metaBuilder.MakeFieldAccess(fieldDef); }

		Backend::ILVariable& MakeTemp(SymbolDef* type) { return metaBuilder.MakeTemp(type); }

		Backend::ILVariable& MakeTemp(Backend::TypeLayout* type) { return metaBuilder.MakeTemp(type); }

		Backend::PointerLayout* MakeAddrType(SymbolDef* elementType) { return metaBuilder.MakeAddrType(elementType); }
	};
}

#endif