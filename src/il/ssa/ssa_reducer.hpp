#ifndef SSA_REDUCER_HPP
#define SSA_REDUCER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class SSAReducer : CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		std::unordered_map<ILVarId, ILVarId> phiMap;
		std::unordered_map<ILVarId, ILVarId> varMapping;
		std::unordered_map<ILVarId, size_t> lastUseIndex;
		std::vector<std::queue<ILVarId>> freeTemps;
		std::unordered_set<ILVarId> seenVars;

		ILVarId nextVarId = SSA_VARIABLE_TEMP_FLAG;

		ILVarId GetFinalVarId(ILVarId varId, bool result)
		{
			auto typeId = metadata.GetTempVar(varId).typeId;
			auto& freeTempsQ = freeTemps[typeId];
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

		void Prepare(const ILInstruction& instr, size_t instrIdx)
		{
			if (instr.operandLeft.IsVar())
			{
				lastUseIndex.insert_or_assign(instr.operandLeft.varId, instrIdx);
			}
			if (instr.operandRight.IsVar())
			{
				lastUseIndex.insert_or_assign(instr.operandRight.varId, instrIdx);
			}
			if (instr.operandResult.IsVar())
			{
				seenVars.insert(instr.operandResult.varId & SSA_VERSION_STRIP_MASK);
			}
		}

		void RemapOperand(ILOperand& op, size_t instrIdx, bool isResult)
		{
			if (!op.IsVar()) return;

			ILVarId varId = op.varId;

			if (!IsTempVar(varId)) return;

			if (isResult)
			{
				op.varId = GetFinalVarId(varId, isResult);
			}
			else
			{
				auto it = varMapping.find(varId);
				if (it != varMapping.end())
				{
					op.varId = it->second;
				}
			}

			if (lastUseIndex[varId] != instrIdx) return;

			auto id = varId & SSA_VARIABLE_MASK;

			auto typeId = metadata.GetTempVar(varId).typeId;

			freeTemps[typeId].push(op.varId & SSA_VERSION_STRIP_MASK);
		}

		void RemapOperandsAndResult(ILInstruction& instr, size_t instrIdx)
		{
			RemapOperand(instr.operandLeft, instrIdx, false);
			RemapOperand(instr.operandRight, instrIdx, false);
			RemapOperand(instr.operandResult, instrIdx, true);
		}

		void TryClearVersion(ILOperand& op);
		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		SSAReducer(ILMetadata& metadata, ControlFlowGraph& cfg) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}

		void Reduce();
	};
}

#endif