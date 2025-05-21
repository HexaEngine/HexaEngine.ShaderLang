#ifndef IL_METADATA_HPP
#define IL_METADATA_HPP

#include "pch/ast.hpp"
#include "il_instruction.hpp"
#include "il_temp_var_allocator.hpp"

namespace HXSL
{
	struct ILTypeMetadata
	{
		SymbolDef* def;

		ILTypeMetadata(SymbolDef* def)
			: def(def)
		{
		}
	};

	enum ILVariableFlags : int
	{
		ILVariableFlags_None = 0,
		ILVariableFlags_Reference = 1 << 0,
		ILVariableFlags_LargeObject = 1 << 1,
	};

	DEFINE_FLAGS_OPERATORS(ILVariableFlags, int)

		struct ILVariable
	{
		uint64_t id;
		uint64_t typeId;
		SymbolDef* var;
		ILVariableFlags flags;

		constexpr ILVariable(const uint64_t& id, const uint64_t& typeId, SymbolDef* var, ILVariableFlags flags = ILVariableFlags_None)
			: id(id), typeId(typeId), var(var), flags(flags)
		{
		}

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

	constexpr ILVariable INVALID_VARIABLE = ILVariable(-1, -1, nullptr);

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
		std::vector<uint64_t> params;
	};

	struct ILMetadata
	{
		ILTempVariableAllocator& tempAllocator;

		std::vector<ILTypeMetadata> typeMetadata;
		std::unordered_map<SymbolDef*, uint64_t> typeMap;

		std::vector<ILVariable> variables;
		std::unordered_map<SymbolDef*, uint64_t> varMap;
		ILVariable invalid = INVALID_VARIABLE;

		std::vector<ILCall> functions;
		std::unordered_map<SymbolDef*, uint64_t> funcMap;

		std::vector<PhiMetadata> phiMetadata;

		ILMetadata(ILTempVariableAllocator& tempAllocator) : tempAllocator(tempAllocator)
		{
		}

		void RemoveFunc(uint64_t funcSlot)
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

		uint64_t RegType(SymbolDef* def)
		{
			auto it = typeMap.find(def);
			if (it != typeMap.end())
			{
				return it->second;
			}

			auto idx = typeMetadata.size();
			typeMetadata.push_back(ILTypeMetadata(def));
			typeMap.insert({ def, idx });
			return idx;
		}

		ILVariable& RegVar(uint64_t typeId, SymbolDef* def)
		{
			auto it = varMap.find(def);
			if (it != varMap.end())
			{
				return variables[it->second];
			}

			auto idx = variables.size();

			ILVariable var = ILVariable(idx, typeId, def);

			variables.push_back(var);
			varMap.insert({ def, idx });

			return variables[idx];
		}

		ILVariable& RegVar(SymbolDef* type, SymbolDef* var)
		{
			auto typeId = RegType(type);
			return RegVar(typeId, var);
		}

		uint64_t RegFunc(SymbolDef* def)
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
	};

	class ILMetadataAdapter
	{
	protected:
		ILMetadata& metadata;
	public:
		ILMetadataAdapter(ILMetadata& metadata) : metadata(metadata) {}

		uint64_t RegType(SymbolDef* def) { return metadata.RegType(def); }

		ILVariable& RegVar(uint64_t typeId, SymbolDef* def) { return metadata.RegVar(typeId, def); }

		ILVariable& RegVar(SymbolDef* type, SymbolDef* var) { return metadata.RegVar(type, var); }

		uint64_t RegFunc(SymbolDef* def) { return metadata.RegFunc(def); }

		ILVariable& FindVar(SymbolDef* var) { return metadata.FindVar(var); }

		ILVariable& FindVar(MemberReferenceExpression* expr) { return metadata.FindVar(expr); }

		ILVariable& FindVar(MemberAccessExpression* expr) { return metadata.FindVar(expr); }

		ILFieldAccess MakeFieldAccess(SymbolDef* fieldDef) { return metadata.MakeFieldAccess(fieldDef); }
	};
}

#endif