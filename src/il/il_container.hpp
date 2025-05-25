#ifndef IL_CONTAINER_HPP
#define IL_CONTAINER_HPP

#include "il_instruction.hpp"

namespace HXSL
{
	struct ILContainer : public ilist<ILInstruction>
	{
		ILContainer(BumpAllocator& allocator) : ilist<ILInstruction>(allocator)
		{
		}

		void AddInstr(const ILInstruction& instr)
		{
			append(instr);
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandRight, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			append(ILInstruction(opcode, operandLeft, operandRight, operandResult, opKind));
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			append(ILInstruction(opcode, operandLeft, operandResult, opKind));
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, ILOpKind opKind = ILOpKind_None)
		{
			append(ILInstruction(opcode, operandLeft, opKind));
		}

		void AddInstr(ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			append(ILInstruction(opcode, opKind));
		}
	};

	struct ILDiff
	{
		size_t start = 0;
		int64_t diff = 0;

		ILDiff(const size_t& start, const int64_t& diff)
			: start(start), diff(diff)
		{
		}

		ILDiff() = default;
	};

	class ILContainerAdapter
	{
	protected:
		ILContainer& container;
	public:
		ILContainerAdapter(ILContainer& container) : container(container) {}

		void AddInstr(const ILInstruction& instr)
		{
			container.AddInstr(instr);
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandRight, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			container.AddInstr(opcode, operandLeft, operandRight, operandResult, opKind);
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			container.AddInstr(opcode, operandLeft, operandResult, opKind);
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, ILOpKind opKind = ILOpKind_None)
		{
			container.AddInstr(opcode, operandLeft, opKind);
		}

		void AddInstr(ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			container.AddInstr(opcode, opKind);
		}
	};
}

#endif