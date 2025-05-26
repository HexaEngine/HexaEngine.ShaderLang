#ifndef IL_METADATA_HPP
#define IL_METADATA_HPP

#include "ast_ilgen.hpp"
#include "il_instruction.hpp"
#include "ssa/ssa_instruction.hpp"

namespace HXSL
{
	enum ILVariableFlags : int
	{
		ILVariableFlags_None = 0,
		ILVariableFlags_Reference = 1 << 0,
		ILVariableFlags_LargeObject = 1 << 1,
	};

	DEFINE_FLAGS_OPERATORS(ILVariableFlags, int)

		static ILVariableFlags GetVarTypeFlags(SymbolDef* def)
	{
		bool isLargeObject = true;
		if (auto prim = def->As<Primitive>())
		{
			isLargeObject = prim->GetClass() == PrimitiveClass_Matrix;
		}

		ILVariableFlags flags = ILVariableFlags_None;
		if (isLargeObject)
		{
			flags |= ILVariableFlags_LargeObject | ILVariableFlags_Reference;
		}

		return flags;
	}

	struct ILTypeMetadata
	{
		ILTypeId id;
		SymbolDef* def;

		ILTypeMetadata(const ILTypeId& id, SymbolDef* def)
			: id(id), def(def)
		{
		}

		ILVariableFlags GetVarFlags() const
		{
			return GetVarTypeFlags(def);
		}
	};

	struct ILVariable
	{
		ILVarId id;
		ILTypeId typeId;
		SymbolDef* var;
		ILVariableFlags flags;

		constexpr ILVariable(const ILVarId& id, const ILTypeId& typeId, SymbolDef* var, ILVariableFlags flags = ILVariableFlags_None)
			: id(id), typeId(typeId), var(var), flags(flags)
		{
		}

		ILVariable() : id(-1), typeId(-1), var(nullptr), flags(ILVariableFlags_None) {}

		ILOperand AsOperand() const noexcept
		{
			return ILOperand(ILOperandKind_Variable, id);
		}

		ILOperand AsTypeOperand() const noexcept
		{
			return ILOperand(ILOperandKind_Type, typeId);
		}

		bool HasFlag(ILVariableFlags flag) const noexcept
		{
			return (flags & flag) != 0;
		}

		void SetFlag(ILVariableFlags flag, bool value) noexcept
		{
			if (value)
			{
				flags |= flag;
			}
			else
			{
				flags &= ~flag;
			}
		}

		bool IsReference() const noexcept
		{
			return HasFlag(ILVariableFlags_Reference);
		}

		bool IsLargeObject() const noexcept
		{
			return HasFlag(ILVariableFlags_LargeObject);
		}
	};

	constexpr ILVariable INVALID_VARIABLE_METADATA = ILVariable(-1, -1, nullptr);

	struct ILCall
	{
		SymbolDef* func;

		ILCall(SymbolDef* func)
			: func(func)
		{
		}
	};

	struct PhiMetadata
	{
		ILVarId varId;
		std::vector<ILVarId> params;

		PhiMetadata(const ILVarId& varId, const std::vector<ILVarId>& params = {}) : varId(varId), params(params)
		{
		}

		PhiMetadata() : varId(-1)
		{
		}
	};

	using ILMemoryLocation = uint64_t;

	struct MemoryLocationMetadata
	{
	};

	struct ILMetadata
	{
		std::string unknownString = "Unknown";
		ILVariable invalid = INVALID_VARIABLE_METADATA;
		std::vector<ILTypeMetadata> typeMetadata;
		std::unordered_map<SymbolDef*, ILTypeId> typeMap;

		std::vector<ILVariable> variables;
		std::vector<ILVariable> tempVariables;
		std::unordered_map<SymbolDef*, ILVarId> varMap;

		std::vector<ILCall> functions;
		std::unordered_map<SymbolDef*, ILFuncId> funcMap;

		std::vector<PhiMetadata> phiMetadata;

		ILMetadata()
		{
		}

		void RemoveFunc(ILFuncId funcSlot)
		{
			auto& funcCall = functions[funcSlot];
			funcMap.erase(funcCall.func);
			funcCall.func = nullptr;
		}

		void RemoveFunc(SymbolDef* def)
		{
			auto it = funcMap.find(def);
			if (it != funcMap.end())
			{
				RemoveFunc(it->second);
			}
		}

		ILTypeId RegType(SymbolDef* def)
		{
			auto it = typeMap.find(def);
			if (it != typeMap.end())
			{
				return it->second;
			}

			auto idx = typeMetadata.size();
			typeMetadata.push_back(ILTypeMetadata(idx, def));
			typeMap.insert({ def, idx });
			return idx;
		}

		ILVariable& RegVar(ILTypeId typeId, SymbolDef* def)
		{
			auto it = varMap.find(def);
			if (it != varMap.end())
			{
				return variables[it->second];
			}

			auto idx = variables.size();

			auto& type = typeMetadata[typeId];
			ILVariable var = ILVariable(idx, typeId, def, type.GetVarFlags());

			variables.push_back(var);
			varMap.insert({ def, idx });

			return variables[idx];
		}

		ILVariable& RegVar(SymbolDef* type, SymbolDef* var)
		{
			auto typeId = RegType(type);
			return RegVar(typeId, var);
		}

		ILVariable& RegTempVar(ILTypeId typeId)
		{
			auto idx = tempVariables.size();
			auto& type = typeMetadata[typeId];

			ILVariable var = ILVariable(idx | SSA_VARIABLE_TEMP_FLAG, typeId, nullptr, type.GetVarFlags());
			tempVariables.push_back(var);

			return tempVariables[idx];
		}

		ILVariable& RegTempVar(SymbolDef* type)
		{
			auto typeId = RegType(type);
			return RegTempVar(typeId);
		}

		ILVariable& GetTempVar(ILVarId varId)
		{
			auto slot = varId & SSA_VARIABLE_MASK;
			return tempVariables[slot];
		}

		ILFuncId RegFunc(SymbolDef* def)
		{
			auto it = funcMap.find(def);
			if (it != funcMap.end())
			{
				return it->second;
			}

			auto idx = functions.size();
			functions.push_back(ILCall(def));
			funcMap.insert({ def, idx });
			return idx;
		}

		ILVariable& FindVar(SymbolDef* var)
		{
			auto it = varMap.find(var);
			if (it != varMap.end())
			{
				return variables[it->second];
			}
			return invalid;
		}

		ILVariable& FindVar(MemberReferenceExpression* expr)
		{
			return FindVar(expr->GetSymbolRef()->GetDeclaration());
		}

		ILVariable& FindVar(MemberAccessExpression* expr)
		{
			return FindVar(expr->GetSymbolRef()->GetDeclaration());
		}

		ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef)
		{
			auto field = fieldDef->As<Field>();
			auto type = field->GetParent()->As<Type>();
			ILFieldAccess access{};
			access.typeId = static_cast<uint32_t>(RegType(type));
			access.fieldId = static_cast<uint32_t>(type->GetFieldOffset(field));
			return access;
		}

		std::string_view GetTypeName(ILTypeId typeId) const
		{
			if (typeId >= typeMetadata.size())
			{
				return unknownString;
			}

			return typeMetadata[typeId].def->GetFullyQualifiedName();
		}

		std::string_view GetFieldName(ILFieldAccess access) const
		{
			auto typeId = access.typeId;
			if (typeId >= typeMetadata.size())
			{
				return unknownString;
			}

			auto fieldId = access.fieldId;
			auto def = typeMetadata[typeId].def;

			if (auto struct_ = def->As<Struct>())
			{
				auto& fields = struct_->GetFields();
				if (fieldId >= fields.size())
				{
					return unknownString;
				}
				return fields[fieldId]->GetName();
			}

			return unknownString;
		}

		std::string_view GetFuncName(ILFuncId funcId) const
		{
			if (funcId >= functions.size())
			{
				return unknownString;
			}

			return functions[funcId].func->GetFullyQualifiedName();
		}

		std::string_view GetVarTypeName(ILVarId varId) const
		{
			auto id = varId.var.id;
			if (varId.var.temp)
			{
				if (id >= tempVariables.size())
				{
					return unknownString;
				}

				auto& var = tempVariables[id];
				return GetTypeName(var.typeId);
			}
			else
			{
				if (id >= variables.size())
				{
					return unknownString;
				}

				auto& var = variables[id];
				return GetTypeName(var.typeId);
			}
		}
	};

	class ILMetadataAdapter
	{
	protected:
		ILMetadata& metadata;
	public:
		ILMetadataAdapter(ILMetadata& metadata) : metadata(metadata) {}

		ILTypeId RegType(SymbolDef* def) { return metadata.RegType(def); }

		ILVariable& RegVar(ILTypeId typeId, SymbolDef* def) { return metadata.RegVar(typeId, def); }

		ILVariable& RegVar(SymbolDef* type, SymbolDef* var) { return metadata.RegVar(type, var); }

		ILFuncId RegFunc(SymbolDef* def) { return metadata.RegFunc(def); }

		ILVariable& FindVar(SymbolDef* var) { return metadata.FindVar(var); }

		ILVariable& FindVar(MemberReferenceExpression* expr) { return metadata.FindVar(expr); }

		ILVariable& FindVar(MemberAccessExpression* expr) { return metadata.FindVar(expr); }

		ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef) { return metadata.MakeFieldAccess(fieldDef); }
	};

	struct ILTempVariableAllocator
	{
		ILMetadata& metadata;

		ILTempVariableAllocator(ILMetadata& metadata) : metadata(metadata) {}

		ILVarId Alloc(SymbolDef* type)
		{
			return metadata.RegTempVar(type).id;
		}

		void Free(ILVarId reg)
		{
			if (reg == INVALID_VARIABLE) return;
		}
	};
}

#endif