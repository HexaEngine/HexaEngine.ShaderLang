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
			void AddInstr(ILOpCode opcode, ILVarId result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				container.append_move(allocator.Alloc<T>(allocator, opcode, result, factory(std::forward<Operands>(operands))...));
			}

			template<typename T, typename... Operands>
			void AddInstrO(const ILVarId& result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				container.append_move(allocator.Alloc<T>(allocator, result, factory(std::forward<Operands>(operands))...));
			}

			template<typename T, typename... Operands>
			void AddInstrNO(ILOpCode opcode, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				container.append_move(allocator.Alloc<T>(allocator, opcode, factory(std::forward<Operands>(operands))...));
			}

			template<typename T, typename... Operands>
			void AddInstrONO(Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				OperandFactory factory{ allocator };
				container.append_move(allocator.Alloc<T>(allocator, factory(std::forward<Operands>(operands))...));
			}

			void AddBasicInstr(ILOpCode opcode)
			{
				container.append_move(allocator.Alloc<BasicInstr>(allocator, opcode));
			}

			template<typename U>
			void AddStoreInstr(const ILVariable& dst, U&& src)
			{
				if (!dst.IsReference())
				{
					//OpCode_Move
					return;
				}
				OperandFactory factory{ allocator };
				AddInstrONO<StoreInstr>(dst.id, factory(std::forward<U>(src)));
			}

			template<typename U>
			void AddLoadInstr(const ILVariable& src, U&& dst)
			{
				if (!src.IsReference())
				{
					//OpCode_Move
					return;
				}
				OperandFactory factory{ allocator };
				AddInstrO<LoadInstr>(src.id, factory(std::forward<U>(dst)));
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