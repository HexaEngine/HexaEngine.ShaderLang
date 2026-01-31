#ifndef IL_METADATA_HPP
#define IL_METADATA_HPP

#include "core/module.hpp"
#include "instruction.hpp"
#include "ssa/ssa_instruction.hpp"
#include "utils/macros.hpp"

namespace HXSL
{
	struct Stream;

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
			using ILTypeId = uint32_t;
			ILTypeId id;
			TypeLayout* def;

			ILTypeMetadata(ILTypeId id, TypeLayout* def) : id(id), def(def)
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
			using ILFuncCallId = uint32_t;
			ILFuncCallId id;
			FunctionLayout* func;
			std::vector<CallInstr*> callSites;

			ILFuncCallMetadata(ILFuncCallId id, FunctionLayout* func) : id(id), func(func)
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
			using TypeMap = dense_map<TypeLayout*, ILType>;
			BumpAllocator& allocator;
			std::string unknownString = "Unknown";
			ILVariable invalid = INVALID_VARIABLE_METADATA;
			std::vector<ILType> typeMetadata;
			TypeMap typeMap;

			std::vector<ILVariable> variables;
			std::vector<ILVariable> tempVariables;

			std::vector<ILFuncCall> functions;
			dense_map<FunctionLayout*, ILFuncCall> funcMap;

			std::vector<PhiInstr*> phiNodes;

			ILMetadata(BumpAllocator& allocator) : allocator(allocator)
			{
			}

			void CopyFrom(const ILMetadata& other)
			{
				for (auto& typeMeta : other.typeMetadata)
				{
					RegType(typeMeta->def);
				}
				for (auto& var : other.variables)
				{
					CloneVar(var.id, var);
				}
				for (auto& tempVar : other.tempVariables)
				{
					RegTempVar(tempVar.typeId);
				}
				for (auto& funcCall : other.functions)
				{
					RegFunc(funcCall->func);
				}
				for (auto& phi : other.phiNodes)
				{
					phiNodes.push_back(phi);
				}
			}

			void Clear()
			{
				typeMetadata.clear();
				typeMap.clear();
				variables.clear();
				tempVariables.clear();
				functions.clear();
				funcMap.clear();
				phiNodes.clear();
			}

			void RemoveFunc(ILFuncCall funcCall)
			{
				auto id = funcCall->id;
				auto idx = static_cast<size_t>(id);
				funcCall->callSites.clear();
				for (size_t i = idx + 1; i < functions.size(); ++i)
				{
					functions[i]->id--;
				}
				funcMap.erase(funcCall->func);
				functions.erase(functions.begin() + idx);
			}

			void RemoveFunc(FunctionLayout* def)
			{
				auto it = funcMap.find(def);
				if (it != funcMap.end())
				{
					RemoveFunc(it->second);
				}
			}

			ILVariable& CloneVar(ILVarId varId, const ILVariable& var)
			{
				auto type = var.typeId;
				auto newType = RegType(type->def);

				if (varId.temp())
				{
					return RegTempVar(type);
				}
				else
				{
					return RegVar(type);
				}
			}

			ILType RegType(TypeLayout* def)
			{
				auto it = typeMap.find(def);
				if (it != typeMap.end())
				{
					return it->second;
				}
				auto id = static_cast<ILTypeMetadata::ILTypeId>(typeMetadata.size());
				auto meta = allocator.Alloc<ILTypeMetadata>(id, def);
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

				auto id = static_cast<ILFuncCallMetadata::ILFuncCallId>(functions.size());
				auto meta = allocator.Alloc<ILFuncCallMetadata>(id, def);
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
					if (fieldId >= fields.size())
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

			ILType GetTypeById(ILTypeMetadata::ILTypeId typeId) const
			{
				if (typeId >= typeMetadata.size())
				{
					return nullptr;
				}
				return typeMetadata[typeId];
			}

			ILFuncCall GetFuncById(ILFuncCallMetadata::ILFuncCallId funcId) const
			{
				if (funcId >= functions.size())
				{
					return nullptr;
				}
				return functions[funcId];
			}

			void Write(Stream* stream, ModuleWriterContext& context);
			void Read(Stream* stream, ModuleReaderContext& context);
		};
	}
}

#endif