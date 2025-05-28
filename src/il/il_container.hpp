#ifndef IL_CONTAINER_HPP
#define IL_CONTAINER_HPP

#include "il_instruction.hpp"
#include "operand_factory.hpp"

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

		template<typename... Operands>
		void AddInstr(ILOpCode opcode, Operands&&... operands)
		{
			emplace_append(opcode, std::forward<Operands>(operands)...);
		}

		void AddInstr(ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			emplace_append(opcode, opKind);
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
		BumpAllocator& allocator;
		ILContainer& container;

	public:
		ILContainerAdapter(ILContainer& container) : allocator(container.get_allocator()), container(container) {}

		void AddInstr(const ILInstruction& instr)
		{
			container.AddInstr(instr);
		}

		template<typename... Operands>
		void AddInstr(ILOpCode opcode, ILVarId result, Operands&&... operands)
		{
			OperandFactory factory{ allocator };
			container.emplace_append(opcode, result, factory(std::forward<Operands>(operands))...);
		}

		template<typename... Operands>
		void AddInstrNO(ILOpCode opcode, Operands&&... operands)
		{
			OperandFactory factory{ allocator };
			container.emplace_append(opcode, INVALID_VARIABLE, factory(std::forward<Operands>(operands))...);
		}

		void AddInstr(ILOpCode opcode, ILOpKind opKind = ILOpKind_None)
		{
			container.emplace_append(opcode, opKind);
		}
	};
}

#endif