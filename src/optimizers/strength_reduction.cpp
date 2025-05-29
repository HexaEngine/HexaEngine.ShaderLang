#include "strength_reduction.hpp"

namespace HXSL
{
	DEFINE_IMM_COMP(IsTwo, 2);

	void StrengthReduction::MulDivReduce(BinaryInstr& instr)
	{
		auto opcode = instr.GetOpCode();
		if (opcode == OpCode_Multiply && IsTwo(instr.GetRHS()))
		{
			changed = true;
			instr.OverwriteOpCode(OpCode_Add);
			instr.GetRHS() = instr.GetLHS();
			return;
		}

		auto immR = dyn_cast<Constant>(instr.GetRHS());
		if (!immR) return;

		uint64_t val = 0;
		auto& imm = immR->imm();
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

		if (opcode == OpCode_Multiply)
		{
			changed = true;
			instr.OverwriteOpCode(OpCode_BitwiseShiftLeft);
			instr.GetRHS() = context->MakeConstant(Cast(instr, instr.GetResult(), shiftNum));
		}
		else if (opcode == OpCode_Divide)
		{
			changed = true;
			instr.OverwriteOpCode(OpCode_BitwiseShiftRight);
			instr.GetRHS() = context->MakeConstant(Cast(instr, instr.GetResult(), shiftNum));
		}
	}

	void StrengthReduction::Visit(size_t index, BasicBlock& node, EmptyCFGContext& context)
	{
		for (auto& instr : node)
		{
			switch (instr.GetOpCode())
			{
			case OpCode_Multiply:
			case OpCode_Divide:
			{
				MulDivReduce(*cast<BinaryInstr>(&instr));
			}
			break;
			}
		}
	}
}