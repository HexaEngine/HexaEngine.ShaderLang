#include "dead_code_eliminator.hpp"

namespace HXSL
{
	void DeadCodeEliminator::ProcessOperand(ILOperand& op)
	{
		if (op.IsVar())
		{
			usedVars.insert(op.varId);
		}
	}

	void DeadCodeEliminator::ProcessInstr(ILInstruction& instr, size_t idx, bool protectedInstr)
	{
		if (instr.opcode == OpCode_Phi)
		{
			auto& phiNode = metadata.phiMetadata[instr.operandLeft.varId];
			for (auto& usedVarId : phiNode.params)
			{
				usedVars.insert(usedVarId);
			}
		}

		ProcessOperand(instr.operandLeft);
		ProcessOperand(instr.operandRight);

		if (protectedInstr) return;
		auto& op = instr.operandResult;
		if (op.IsVar() && usedVars.find(op.varId) == usedVars.end())
		{
			deadVars.insert(op.varId);
			DiscardInstr(idx);
		}
	}

	void DeadCodeEliminator::VisitClose(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		const size_t n = instructions.size();

		bool protectedInstr = false;
		for (size_t i = n; i-- != 0; )
		{
			auto& instr = instructions[i];
			protectedInstr |= instr.opcode == OpCode_Store;

			ProcessInstr(instr, i, protectedInstr);
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
}