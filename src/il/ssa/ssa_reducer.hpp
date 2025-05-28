#ifndef SSA_REDUCER_HPP
#define SSA_REDUCER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class SSAReducer : CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		std::unordered_map<ILVarId, ILVarId> phiMap;
		std::unordered_map<ILVarId, ILVarId> varMapping;
		std::unordered_map<ILVarId, const ILInstruction*> lastUseIndex;
		std::vector<std::queue<ILVarId>> freeTemps;
		std::unordered_set<ILVarId> seenVars;

		ILVarId nextVarId = SSA_VARIABLE_TEMP_FLAG;

		ILVarId GetFinalVarId(ILVarId varId, bool result)
		{
			auto typeId = metadata.GetTempVar(varId).typeId;
			auto& freeTempsQ = freeTemps[typeId.value];
			if (!freeTempsQ.empty())
			{
				ILVarId finalId = freeTempsQ.front();
				freeTempsQ.pop();
				varMapping[varId] = finalId;
				return finalId;
			}

			ILVarId finalId = metadata.RegTempVar(typeId).id;
			varMapping[varId] = finalId;
			return finalId;
		}

		void Prepare(const ILInstruction& instr)
		{
			if (auto var = dyn_cast<Variable>(instr.operandLeft))
			{
				lastUseIndex.insert_or_assign(var->varId, &instr);
			}
			if (auto var = dyn_cast<Variable>(instr.operandRight))
			{
				lastUseIndex.insert_or_assign(var->varId, &instr);
			}
			if (auto var = dyn_cast<Variable>(instr.operandResult))
			{
				seenVars.insert(var->varId & SSA_VERSION_STRIP_MASK);
			}
		}

		void RemapOperand(Operand* op, ILInstruction* instr, bool isResult)
		{
			auto var = dyn_cast<Variable>(op);
			if (!var) return;

			ILVarId varId = var->varId;

			if (!IsTempVar(varId)) return;

			if (isResult)
			{
				var->varId = GetFinalVarId(varId, isResult);
			}
			else
			{
				auto it = varMapping.find(varId);
				if (it != varMapping.end())
				{
					var->varId = it->second;
				}
			}

			if (lastUseIndex[varId] != instr) return;

			auto id = varId & SSA_VARIABLE_MASK;

			auto typeId = metadata.GetTempVar(varId).typeId;

			freeTemps[typeId.value].push(var->varId & SSA_VERSION_STRIP_MASK);
		}

		void RemapOperandsAndResult(ILInstruction& instr)
		{
			RemapOperand(instr.operandLeft, &instr, false);
			RemapOperand(instr.operandRight, &instr, false);
			RemapOperand(instr.operandResult, &instr, true);
		}

		void TryClearVersion(Operand* op);
		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		SSAReducer(ILMetadata& metadata, ControlFlowGraph& cfg) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}

		void Reduce();
	};
}

#endif