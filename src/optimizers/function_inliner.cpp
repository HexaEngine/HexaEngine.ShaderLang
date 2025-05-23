#include "function_inliner.hpp"

namespace HXSL
{
	/*
	bool FunctionInliner::TryMapOperand(ILOperand& operand)
	{
		if (operand.kind == ILOperandKind_Register)
		{
			auto it = registerMap.find(operand.reg);
			if (it != registerMap.end())
			{
				operand.reg = it->second;
				return true;
			}
			return false;
		}
		else if (operand.kind == ILOperandKind_Variable)
		{
			auto it = variableMap.find(operand.varId);
			if (it != variableMap.end())
			{
				operand.varId = it->second;
				return true;
			}
			return false;
		}
		return true;
	}

	ILOperand FunctionInliner::MapOperand(const ILOperand& operand)
	{
		ILOperand copy = operand;
		TryMapOperand(copy);
		return copy;
	}

	ILInstruction FunctionInliner::MapInstr(const ILInstruction& instr)
	{
		ILInstruction copy = instr;
		if (!TryMapOperand(copy.operandLeft))
		{
			copy.operandLeft = AddMapping(instr.operandLeft);
		}
		if (!TryMapOperand(copy.operandRight))
		{
			copy.operandRight = AddMapping(instr.operandRight);
		}
		if (!TryMapOperand(copy.operandResult))
		{
			copy.operandResult = AddMapping(instr.operandResult);
		}

		return copy;
	}

	ILOperand FunctionInliner::AddMapping(const ILOperand& op)
	{
		if (op.kind == ILOperandKind_Register)
		{
			auto newReg = builder.GetTempAllocator().Alloc();
			registerMap.insert({ op.reg, newReg });
			return newReg;
		}
		else if (op.kind == ILOperandKind_Variable)
		{
		}
		return op;
	}

	void FunctionInliner::Inline(ILContext& ctx, uint64_t funcSlot)
	{
		auto& container = builder.GetContainer();
		ILContainer outContainer;
		std::vector<size_t> params;
		std::vector<ILDiff> diffs;
		for (size_t i = 0; i < container.size(); i++)
		{
			auto& instr = container[i];
			if (instr.opcode == OpCode_StoreParam)
			{
				params.push_back(i);
				continue;
			}
			if (instr.opcode == OpCode_Call)
			{
				if (instr.operandLeft.kind == ILOperandKind_Func && instr.operandLeft.varId == funcSlot)
				{
					ILOperand returnDest = instr.operandResult;
					size_t paramIndex = 0;
					auto& otherContainer = ctx.builder.GetContainer();
					for (auto& otherInstr : otherContainer.instructions)
					{
						if (otherInstr.IsOp(OpCode_LoadParam))
						{
							auto& param = container[params[paramIndex]];
							outContainer.AddInstr(OpCode_Move, param.operandLeft, AddMapping(otherInstr.operandLeft));
							continue;
						}
						else if (otherInstr.IsOp(OpCode_Return))
						{
							outContainer.AddInstr(OpCode_Move, MapOperand(otherInstr.operandLeft), returnDest);
							continue;
						}

						outContainer.AddInstr(MapInstr(otherInstr));
					}

					size_t start = params.size() > 0 ? params[0] : i;
					int64_t len = static_cast<int64_t>((i + 1) - start);
					int64_t diff = otherContainer.size() - len;
					if (diff != 0)
					{
						diffs.push_back(ILDiff(start, diff));
					}
					params.clear();
					continue;
				}
				for (auto param : params) { outContainer.AddInstr(container[param]); }
				params.clear();
			}

			outContainer.AddInstr(instr);
		}

		// we could do a heuristic here.
		container.instructions = std::move(outContainer.instructions);
		auto& jumpTable = builder.GetJumpTable();
		jumpTable.Relocate(diffs);
		auto& metadata = builder.GetMetadata();
		metadata.RemoveFunc(funcSlot);
	}
	*/
}