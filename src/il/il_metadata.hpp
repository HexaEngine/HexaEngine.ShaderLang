#ifndef IL_METADATA_HPP
#define IL_METADATA_HPP

#include "core/module.hpp"
#include "instruction.hpp"
#include "ssa/ssa_instruction.hpp"

namespace HXSL
{
	namespace Backend
	{
		enum ILVariableFlags : int
		{
			ILVariableFlags_None = 0,
			ILVariableFlags_Reference = 1 << 0,
			ILVariableFlags_LargeObject = 1 << 1,
		};

		DEFINE_FLAGS_OPERATORS(ILVariableFlags, int)

			inline static ILVariableFlags GetVarTypeFlags(TypeLayout* def)
		{
			ILVariableFlags flags = ILVariableFlags_None;
			if (auto prim = dyn_cast<PrimitiveLayout>(def))
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

			if (isa<PointerLayout>(def))
			{
				flags |= ILVariableFlags_Reference;
			}

			return flags;
		}

		class ILTypeMetadata
		{
		public:
			TypeLayout* def;

			ILTypeMetadata(TypeLayout* def) : def(def)
			{
			}

			ILVariableFlags GetVarTypeFlags() const
			{
				return HXSL::Backend::GetVarTypeFlags(def);
			}
		};

		struct ILVariable
		{
			ILVarId id;
			ILType typeId;
			ILVariableFlags flags;

			constexpr ILVariable(const ILVarId& id, ILType typeId, ILVariableFlags flags = ILVariableFlags_None)
				: id(id), typeId(typeId), flags(flags)
			{
			}

			ILVariable() : id(-1), typeId(nullptr), flags(ILVariableFlags_None) {}

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

		constexpr ILVariable INVALID_VARIABLE_METADATA = ILVariable(ILVarId(-1), ILType(nullptr));

		class ILFuncCallMetadata
		{
		public:
			FunctionLayout* func;

			ILFuncCallMetadata(FunctionLayout* func) : func(func)
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
			std::unordered_map<TypeLayout*, ILType> typeMap;

			std::vector<ILVariable> variables;
			std::vector<ILVariable> tempVariables;

			std::vector<ILFuncCall> functions;
			std::unordered_map<FunctionLayout*, ILFuncCall> funcMap;

			std::vector<PhiInstr*> phiNodes;

			ILMetadata(BumpAllocator& allocator) : allocator(allocator)
			{
			}

			void RemoveFunc(ILFuncCall funcCall)
			{
				funcMap.erase(funcCall->func);
				funcCall->func = nullptr;
			}

			void RemoveFunc(FunctionLayout* def)
			{
				auto it = funcMap.find(def);
				if (it != funcMap.end())
				{
					RemoveFunc(it->second);
				}
			}

			ILType RegType(TypeLayout* def)
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

			ILVariable& RegVar(ILType type)
			{
				auto idx = variables.size();

				ILVariable var = ILVariable(idx, type, type->GetVarTypeFlags());
				variables.push_back(var);
				return variables[idx];
			}

			ILVariable& RegVar(TypeLayout* type)
			{
				auto typeId = RegType(type);
				return RegVar(typeId);
			}

			ILVariable& RegTempVar(ILType type)
			{
				auto idx = tempVariables.size();

				ILVariable var = ILVariable(idx | SSA_VARIABLE_TEMP_FLAG, type, type->GetVarTypeFlags());
				tempVariables.push_back(var);

				return tempVariables[idx];
			}

			ILVariable& RegTempVar(TypeLayout* type)
			{
				auto typeId = RegType(type);
				return RegTempVar(typeId);
			}

			ILVariable& GetTempVar(ILVarId varId)
			{
				auto slot = varId & SSA_VARIABLE_MASK;
				return tempVariables[slot];
			}

			ILFuncCall RegFunc(FunctionLayout* def)
			{
				auto it = funcMap.find(def);
				if (it != funcMap.end())
				{
					return ILFuncCall(it->second);
				}

				auto meta = allocator.Alloc<ILFuncCallMetadata>(def);
				functions.push_back(meta);
				funcMap.insert({ def, meta });
				return meta;
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

			ILFieldAccess MakeFieldAccess(FieldLayout* field)
			{
				auto type = cast<StructLayout>(field->GetParent());
				auto typeId = RegType(type);
				auto fieldId = ILFieldId(static_cast<uint32_t>(type->GetFieldOffset(field)));
				return ILFieldAccess(typeId, fieldId);
			}

			StringSpan GetTypeName(ILType type) const
			{
				if (type == nullptr)
				{
					return unknownString;
				}

				return type->def->GetName();
			}

			StringSpan GetFieldName(ILFieldAccess access) const
			{
				auto type = access.typeId;
				if (type == nullptr)
				{
					return unknownString;
				}

				auto fieldId = access.fieldId.value;
				auto def = type->def;

				if (auto struct_ = dyn_cast<StructLayout>(def))
				{
					auto& fields = struct_->GetFields();
					if (fieldId >= fields.length)
					{
						return unknownString;
					}
					return fields[fieldId]->GetName();
				}

				return unknownString;
			}

			StringSpan GetFuncName(ILFuncCall funcCall) const
			{
				if (funcCall == nullptr)
				{
					return unknownString;
				}

				return funcCall->func->GetName();
			}

			StringSpan GetVarTypeName(ILVarId varId) const
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
	}
}

#endif