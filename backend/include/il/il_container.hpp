#ifndef IL_CONTAINER_HPP
#define IL_CONTAINER_HPP

#include "instruction.hpp"
#include "operand_factory.hpp"
#include "il_metadata.hpp"

namespace HXSL
{
	namespace Backend
	{
		struct ILContainer : public ilist<Instruction>
		{
			ILContainer(BumpAllocator& allocator) : ilist<Instruction>(allocator)
			{
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

			template<typename T, typename... Operands>
			T* AddInstr(ILOpCode opcode, ILVarId result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				auto* instr = allocator.Alloc<T>(allocator, opcode, result, factory(std::forward<Operands>(operands))...);
				container.append_move(instr);
				return instr;
			}

			template<typename T, typename... Operands>
			T* AddInstrO(const ILVarId& result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				auto* instr = allocator.Alloc<T>(allocator, result, factory(std::forward<Operands>(operands))...);
				container.append_move(instr);
				return instr;
			}

			template<typename T, typename... Operands>
			T* AddInstrNO(ILOpCode opcode, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				auto* instr = allocator.Alloc<T>(allocator, opcode, factory(std::forward<Operands>(operands))...);
				container.append_move(instr);
				return instr;
			}

			template<typename T, typename... Operands>
			T* AddInstrONO(Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				auto* instr = allocator.Alloc<T>(allocator, factory(std::forward<Operands>(operands))...);
				container.append_move(instr);
				return instr;
			}

			BasicInstr* AddBasicInstr(ILOpCode opcode)
			{
				auto* instr = allocator.Alloc<BasicInstr>(allocator, opcode);
				container.append_move(instr);
				return instr;
			}

			template<typename U>
			void AddStoreInstr(const ILVariable& dst, U&& src)
			{
				OperandFactory factory{ allocator };
				if (!dst.IsReference())
				{
					AddInstrO<MoveInstr>(dst.id, factory(std::forward<U>(src)));
					return;
				}

				AddInstrONO<StoreInstr>(dst.id, factory(std::forward<U>(src)));
			}

			void AddLoadInstr(const ILVariable& src, const ILVarId& dst)
			{
				OperandFactory factory{ allocator };
				if (!src.IsReference())
				{
					AddInstrO<MoveInstr>(dst, factory(src));
					return;
				}

				AddInstrO<LoadInstr>(dst, factory(src));
			}

			template<typename U>
			void AddInstr(U&& v)
			{
				static_assert(std::is_base_of_v<Instruction, U>, "U must derive from Instruction");
				container.append_move(allocator.Alloc<U>(std::forward<U>(v)));
			}

			template<typename U>
			void AddInstr(const U& v)
			{
				static_assert(std::is_base_of_v<Instruction, U>, "U must derive from Instruction");
				container.append_move(allocator.Alloc<U>(v));
			}
		};
	}
}

#endif