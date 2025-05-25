#include "ssa_reducer.hpp"

namespace HXSL
{
	void SSAReducer::TryClearVersion(ILOperand& op)
	{
		if (!op.IsVar()) return;
		auto it = phiMap.find(op.varId);
		if (it != phiMap.end())
		{
			op.varId = it->second;
		}
	}

	void SSAReducer::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		lastUseIndex.clear();

		auto& instructions = node.instructions;
		const size_t n = instructions.size();
		for (auto& instr : instructions)
		{
			if (instr.opcode == OpCode_Phi)
			{
				DiscardInstr(instr);
				continue;
			}

			TryClearVersion(instr.operandLeft);
			TryClearVersion(instr.operandRight);
			TryClearVersion(instr.operandResult);

			Prepare(instr);
		}

		for (auto& p : seenVars)
		{
			ILVarId varId = p;
			if (!IsTempVar(varId)) continue;

			auto typeId = metadata.GetTempVar(varId).typeId;
			freeTemps[typeId].push(varId);
		}

		for (auto& instr : instructions)
		{
			RemapOperandsAndResult(instr);
		}

		DiscardMarkedInstructs(node);
	}

	void SSAReducer::Reduce()
	{
		for (auto& phi : metadata.phiMetadata)
		{
			for (auto& p : phi.params)
			{
				phiMap.insert({ p, phi.varId });
			}
		}
		freeTemps.resize(metadata.typeMetadata.size());
		Traverse();
	}
}