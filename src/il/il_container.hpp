#ifndef IL_CONTAINER_HPP
#define IL_CONTAINER_HPP

#include "il_instruction.hpp"

namespace HXSL
{
	struct ILContainer
	{
		std::vector<ILInstruction> instructions;

		size_t size() const { return instructions.size(); }

		const ILInstruction& operator[](size_t index) const
		{
			return instructions[index];
		}

		ILInstruction& operator[](size_t index)
		{
			return instructions[index];
		}

		void AddInstr(const ILInstruction& instr)
		{
			instructions.push_back(instr);
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandRight, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			instructions.push_back(ILInstruction(opcode, operandLeft, operandRight, operandResult, opKind));
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			instructions.push_back(ILInstruction(opcode, operandLeft, operandResult, opKind));
		}

		void AddInstr(ILOpCode opcode, const ILOperand& operandLeft, ILOpKind opKind = ILOpKind_None)
		{
			instructions.push_back(ILInstruction(opcode, operandLeft, opKind));
		}

		void AddInstr(ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			instructions.push_back(ILInstruction(opcode, opKind));
		}

		void InsertInstr(size_t index, const ILInstruction& instr)
		{
			instructions.insert(instructions.begin() + index, instr);
		}

		void InsertInstr(size_t index, ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandRight, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			instructions.insert(instructions.begin() + index, ILInstruction(opcode, operandLeft, operandRight, operandResult, opKind));
		}

		void InsertInstr(size_t index, ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None)
		{
			instructions.insert(instructions.begin() + index, ILInstruction(opcode, operandLeft, operandResult, opKind));
		}

		void InsertInstr(size_t index, ILOpCode opcode, const ILOperand& operandLeft, ILOpKind opKind = ILOpKind_None)
		{
			instructions.insert(instructions.begin() + index, ILInstruction(opcode, operandLeft, opKind));
		}

		void InsertInstr(size_t index, ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			instructions.insert(instructions.begin() + index, ILInstruction(opcode, opKind));
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