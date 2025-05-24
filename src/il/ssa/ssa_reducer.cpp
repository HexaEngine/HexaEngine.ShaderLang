#include "ssa_reducer.hpp"

namespace HXSL
{
	static void TryClearVersion(ILOperand& op)
	{
		if (!op.IsVar()) return;
		op.varId &= SSA_VARIABLE_MASK;
	}

	void SSAReducer::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		const size_t n = instructions.size();
		for (size_t i = 0; i < n; i++)
		{
			auto& instr = instructions[i];
			if (instr.opcode == OpCode_Phi)
			{
				DiscardInstr(i);
				continue;
			}

			TryClearVersion(instr.operandLeft);
			TryClearVersion(instr.operandRight);
			TryClearVersion(instr.operandResult);
		}

		DiscardMarkedInstructs(node);
	}

	void SSAReducer::Reduce()
	{
		Traverse();
	}
}