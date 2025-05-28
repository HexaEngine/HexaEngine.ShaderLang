#include "ssa_reducer.hpp"

namespace HXSL
{
	void SSAReducer::TryClearVersion(ILVarId& op)
	{
		auto it = phiMap.find(op);
		if (it != phiMap.end())
		{
			op = it->second;
		}
	}

	void SSAReducer::TryClearVersion(Value* op)
	{
		auto var = dyn_cast<Variable>(op);
		if (!var) return;
		TryClearVersion(var->varId);
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
			if (instr.HasResult())
			{
				TryClearVersion(instr.result);
			}

			Prepare(instr);
		}

		for (auto& p : seenVars)
		{
			ILVarId varId = p;
			if (!IsTempVar(varId)) continue;

			auto typeId = metadata.GetTempVar(varId).typeId;
			freeTemps[typeId.value].push(varId);
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