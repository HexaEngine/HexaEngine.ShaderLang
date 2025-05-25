#include "strength_reduction.hpp"

namespace HXSL
{
	DEFINE_IMM_COMP(IsTwo, 2);

	void StrengthReduction::MulDivReduce(ILInstruction& instr)
	{
		if (instr.opcode == OpCode_Multiply && IsTwo(instr.operandRight))
		{
			changed = true;
			instr.opcode = OpCode_Add;
			instr.operandRight = instr.operandLeft;
			return;
		}

		if (!instr.operandRight.IsImm()) return;

		uint64_t val = 0;
		auto imm = instr.operandRight.imm();
		if (imm.IsNegative()) return;
		switch (imm.Kind)
		{
		case NumberType_Int8:   val = static_cast<uint64_t>(imm.i8); break;
		case NumberType_Int16:  val = static_cast<uint64_t>(imm.i16); break;
		case NumberType_Int32:  val = static_cast<uint64_t>(imm.i32); break;
		case NumberType_Int64:  val = static_cast<uint64_t>(imm.i64); break;
		case NumberType_UInt8:  val = imm.u8; break;
		case NumberType_UInt16: val = imm.u16; break;
		case NumberType_UInt32: val = imm.u32; break;
		case NumberType_UInt64: val = imm.u64; break;
		default:
			return;
		}

		if (val <= 1 || (val & (val - 1)) != 0) return;

		int shiftAmount = 0;
		Number shiftNum(shiftAmount);

		if (instr.opcode == OpCode_Multiply)
		{
			changed = true;
			instr.opcode = OpCode_BitwiseShiftLeft;
			instr.operandRight = Cast(shiftNum, instr.opKind);
		}
		else if (instr.opcode == OpCode_Divide)
		{
			changed = true;
			instr.opcode = OpCode_BitwiseShiftRight;
			instr.operandRight = Cast(shiftNum, instr.opKind);
		}
	}

	void StrengthReduction::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		for (auto& instr : instructions)
		{
			switch (instr.opcode)
			{
			case OpCode_Multiply:
			case OpCode_Divide:
			{
				MulDivReduce(instr);
			}
			break;
			}
		}
	}
}