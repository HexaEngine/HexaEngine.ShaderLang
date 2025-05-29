#include "dead_code_eliminator.hpp"

namespace HXSL
{
	void DeadCodeEliminator::ProcessOperand(Value* op)
	{
		if (Operand::IsVar(op))
		{
			usedVars.insert(cast<Variable>(op)->varId);
		}
	}

	void DeadCodeEliminator::ProcessInstr(ILInstruction& instr, bool protectedInstr)
	{
		if (instr.opcode == OpCode_Phi)
		{
			auto& phiNode = metadata.GetPhi(cast<Phi>(instr.operandLeft)->phiId);
			for (auto& usedVarId : phiNode.params)
			{
				usedVars.insert(usedVarId);
			}
		}

		ProcessOperand(instr.operandLeft);
		ProcessOperand(instr.operandRight);

		if (protectedInstr) return;
		if (instr.HasResult())
		{
			if (usedVars.find(instr.result) == usedVars.end())
			{
				deadVars.insert(instr.result);
				DiscardInstr(instr);
			}
		}
	}

	void DeadCodeEliminator::VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		const size_t n = instructions.size();

		bool protectedInstr = false;
		for (auto it = instructions.rbegin(); it != instructions.rend(); ++it)
		{
			auto& instr = *it;
			protectedInstr |= instr.opcode == OpCode_Store;

			ProcessInstr(instr, protectedInstr);
			if (instr.opcode == OpCode_JumpZero || instr.opcode == OpCode_JumpNotZero)
			{
				protectedInstr = true;
			}
			else
			{
				protectedInstr = false;
			}
		}

		DiscardMarkedInstructs(node);
	}

	OptimizerPassResult DeadCodeEliminator::Run()
	{
		changed = false;
		usedVars.clear();

		for (auto& phi : metadata.phiMetadata)
		{
			for (auto& p : phi.params)
			{
				usedVars.insert(p);
			}
		}

		Traverse();
		return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
	}
}