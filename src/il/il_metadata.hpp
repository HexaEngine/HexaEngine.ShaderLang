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

		inline static ILVariableFlags GetVarTypeFlags(SymbolDef* def)
	{
		ILVariableFlags flags = ILVariableFlags_None;
		if (auto prim = def->As<Primitive>())
		{
			if (prim->GetClass() == PrimitiveClass_Matrix)
			{
				flags |= ILVariableFlags_LargeObject | ILVariableFlags_Reference;
			}
		}
		else
		{
			flags |= ILVariableFlags_LargeObject;
		}

		if (auto ptr = def->As<Pointer>())
		{
			flags |= ILVariableFlags_Reference;
		}

		return flags;
	}

	class ILTypeMetadata
	{
	public:
		SymbolDef* def;

		ILTypeMetadata(SymbolDef* def) : def(def)
		{
		}

		ILVariableFlags GetVarTypeFlags() const
		{
			return HXSL::GetVarTypeFlags(def);
		}
	};

	struct ILVariable
	{
		ILVarId id;
		ILType typeId;
		SymbolDef* var;
		ILVariableFlags flags;

		constexpr ILVariable(const ILVarId& id, ILType typeId, SymbolDef* var, ILVariableFlags flags = ILVariableFlags_None)
			: id(id), typeId(typeId), var(var), flags(flags)
		{
		}

		ILVariable() : id(-1), typeId(nullptr), var(nullptr), flags(ILVariableFlags_None) {}

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

		inline operator ILVarId() const
		{
			return id;
		}
	};

	constexpr ILVariable INVALID_VARIABLE_METADATA = ILVariable(ILVarId(-1), ILType(nullptr), nullptr);

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
		BumpAllocator& allocator;
		std::string unknownString = "Unknown";
		ILVariable invalid = INVALID_VARIABLE_METADATA;
		std::vector<ILType> typeMetadata;
		std::unordered_map<SymbolDef*, ILType> typeMap;

		std::vector<ILVariable> variables;
		std::vector<ILVariable> tempVariables;
		std::unordered_map<SymbolDef*, ILVarId> varMap;

		std::vector<ILCall> functions;
		std::unordered_map<SymbolDef*, ILFuncId> funcMap;

		std::vector<PhiInstr*> phiNodes;

		ILMetadata(BumpAllocator& allocator) : allocator(allocator)
		{
		}

		void RemoveFunc(ILFuncId funcSlot)
		{
			auto& funcCall = functions[funcSlot.value];
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

		ILType RegType(SymbolDef* def)
		{
			auto it = typeMap.find(def);
			if (it != typeMap.end())
			{
				return it->second;
			}

			auto meta = allocator.Alloc<ILTypeMetadata>(def);
			typeMetadata.push_back(meta);
			typeMap.insert({ def, meta });
			return meta;
		}

		ILVariable& RegVar(ILType type, SymbolDef* def)
		{
			auto it = varMap.find(def);
			if (it != varMap.end())
			{
				return variables[it->second];
			}

			auto idx = variables.size();

			ILVariable var = ILVariable(idx, type, def, type->GetVarTypeFlags());

			variables.push_back(var);
			varMap.insert({ def, idx });

			return variables[idx];
		}

		ILVariable& RegVar(SymbolDef* type, SymbolDef* var)
		{
			auto typeId = RegType(type);
			return RegVar(typeId, var);
		}

		ILVariable& RegTempVar(ILType type)
		{
			auto idx = tempVariables.size();

			ILVariable var = ILVariable(idx | SSA_VARIABLE_TEMP_FLAG, type, nullptr, type->GetVarTypeFlags());
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

			auto idx = ILFuncId(functions.size());
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

		ILVariable& GetVar(const ILVarId& varId)
		{
			auto id = varId.var.id;
			if (varId.var.temp)
			{
				if (id >= tempVariables.size())
				{
					return invalid;
				}
				return tempVariables[id];
			}
			else
			{
				if (id >= variables.size())
				{
					return invalid;
				}
				return variables[id];
			}
		}

		const ILVariable& GetVar(const ILVarId& varId) const
		{
			auto id = varId.var.id;
			if (varId.var.temp)
			{
				if (id >= tempVariables.size())
				{
					return invalid;
				}
				return tempVariables[id];
			}
			else
			{
				if (id >= variables.size())
				{
					return invalid;
				}
				return variables[id];
			}
		}

		ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef)
		{
			auto field = fieldDef->As<Field>();
			auto type = field->GetParent()->As<Type>();
			auto typeId = RegType(type);
			auto fieldId = ILFieldId(static_cast<uint32_t>(type->GetFieldOffset(field)));
			return ILFieldAccess(typeId, fieldId);
		}

		std::string_view GetTypeName(ILType type) const
		{
			if (type == nullptr)
			{
				return unknownString;
			}

			return type->def->GetFullyQualifiedName();
		}

		std::string_view GetFieldName(ILFieldAccess access) const
		{
			auto type = access.typeId;
			if (type == nullptr)
			{
				return unknownString;
			}

			auto fieldId = access.fieldId.value;
			auto def = type->def;

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
			if (funcId.value >= functions.size())
			{
				return unknownString;
			}

			return functions[funcId.value].func->GetFullyQualifiedName();
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

		ILType RegType(SymbolDef* def) { return metadata.RegType(def); }

		ILVariable& RegVar(ILType typeId, SymbolDef* def) { return metadata.RegVar(typeId, def); }

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

		ILVariable& Alloc(SymbolDef* type)
		{
			return metadata.RegTempVar(type);
		}

		void Free(ILVarId reg)
		{
			if (reg == INVALID_VARIABLE) return;
		}
	};
}

#endif