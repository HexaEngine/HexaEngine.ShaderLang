#ifndef SSA_REDUCER_HPP
#define SSA_REDUCER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class SSAReducer : CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		std::unordered_map<ILVarId, ILVarId> phiMap;
		std::unordered_map<ILVarId, ILVarId> varMapping;
		std::unordered_map<ILVarId, const Instruction*> lastUseIndex;
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

		void Prepare(const Instruction& instr)
		{
			for (auto& operand : instr.GetOperands())
			{
				if (auto var = dyn_cast<Variable>(operand))
				{
					lastUseIndex.insert_or_assign(var->varId, &instr);
				}
			}

			if (auto res = dyn_cast<ResultInstr>(&instr))
			{
				seenVars.insert(res->GetResult() & SSA_VERSION_STRIP_MASK);
			}
		}

		void RemapVar(ILVarId& varId, Instruction* instr, bool isResult)
		{
			if (!IsTempVar(varId)) return;

			ILVarId tmpVarId = varId;

			if (isResult)
			{
				varId = GetFinalVarId(tmpVarId, isResult);
			}
			else
			{
				auto it = varMapping.find(tmpVarId);
				if (it != varMapping.end())
				{
					varId = it->second;
				}
			}

			if (lastUseIndex[tmpVarId] != instr) return;

			auto id = tmpVarId & SSA_VARIABLE_MASK;

			auto typeId = metadata.GetTempVar(tmpVarId).typeId;

			freeTemps[typeId.value].push(varId & SSA_VERSION_STRIP_MASK);
		}

		void RemapOperand(Operand* op, Instruction* instr, bool isResult)
		{
			auto var = dyn_cast<Variable>(op);
			if (!var) return;

			ILVarId varId = var->varId;

			RemapVar(var->varId, instr, isResult);
		}

		void RemapOperandsAndResult(Instruction& instr)
		{
			for (auto& operand : instr.GetOperands())
			{
				RemapOperand(operand, &instr, false);
			}

			if (auto res = dyn_cast<ResultInstr>(&instr))
			{
				RemapVar(res->GetResult(), &instr, true);
			}
		}

		void TryClearVersion(ILVarId& op);
		void TryClearVersion(Operand* op);
		void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

	public:
		SSAReducer(ILMetadata& metadata, ControlFlowGraph& cfg) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}

		void Reduce();
	};
}

#endif