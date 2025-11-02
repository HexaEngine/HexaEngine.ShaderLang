#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "core/config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "core/number.hpp"
#include "utils/hashing.hpp"
#include "utils/ilist.hpp"
#include "utils/static_vector.hpp"
#include "value.hpp"
#include "operands.hpp"

namespace HXSL
{
	namespace Backend
	{
		enum ILOpCode : uint16_t
		{
			OpCode_Noop,			// nop
			OpCode_StackAlloc,		// <dst> alloca <type_id>
			OpCode_Zero,			// <dst> zero <src>
			OpCode_Store,			// sta <src/imm> <dst>
			OpCode_Load,			// <dst> lda <src>
			OpCode_OffsetAddress,	// <dst> offs <src>, <type_id::field_id>
			OpCode_AddressOf,		// <dst> addr <src>
			OpCode_Push,			// push <src>
			OpCode_Pop,				// <dst> pop
			OpCode_Move,			// <dst> mov <src/imm>

			OpCode_Return,			// ret <src/imm/void>

			OpCode_StoreParam,		// <param_id> starg <src/imm>
			OpCode_LoadParam,		// <dst> ldarg <param_id>

			OpCode_StoreRefParam,	// <param_id> strefarg <src>
			OpCode_LoadRefParam,	// <dst> ldrefarg <param_id>

			OpCode_Call,			// <dst> call <func_id>

			OpCode_Jump,			// jmp <label>
			OpCode_JumpZero,		// jz <label>
			OpCode_JumpNotZero,		// jnz <label>

			OpCode_Cast,			// <dst> cast <src/imm>

			OpCode_Discard,			// discard

			OpCode_Phi,				// <dst> phi <phi_id> not a real instruction

			OpCode_Add,					// <dst> add  <src/imm> <src/imm>
			OpCode_Subtract,			// <dst> sub  <src/imm> <src/imm>
			OpCode_Multiply,			// <dst> mul  <src/imm> <src/imm>
			OpCode_Divide,				// <dst> div  <src/imm> <src/imm>
			OpCode_Modulus,				// <dst> rem  <src/imm> <src/imm>
			OpCode_BitwiseShiftLeft,	// <dst> bls  <src/imm> <src/imm>
			OpCode_BitwiseShiftRight,	// <dst> brs  <src/imm> <src/imm>
			OpCode_AndAnd,				// <dst> land <src/imm> <src/imm>
			OpCode_OrOr,				// <dst> lor  <src/imm> <src/imm>
			OpCode_BitwiseAnd,			// <dst> and  <src/imm> <src/imm>
			OpCode_BitwiseOr,			// <dst> or   <src/imm> <src/imm>
			OpCode_BitwiseXor,			// <dst> xor  <src/imm> <src/imm>
			OpCode_LessThan,			// <dst> lt   <src/imm> <src/imm>
			OpCode_LessThanOrEqual,		// <dst> ltq  <src/imm> <src/imm>
			OpCode_GreaterThan,			// <dst> gt   <src/imm> <src/imm>
			OpCode_GreaterThanOrEqual,	// <dst> gtq  <src/imm> <src/imm>
			OpCode_Equal,				// <dst> eq   <src/imm> <src/imm>
			OpCode_NotEqual,			// <dst> neq  <src/imm> <src/imm>

			OpCode_Increment,			// <dst> inc  <src>
			OpCode_Decrement,			// <dst> dec  <src>
			OpCode_LogicalNot,			// <dst> lnot <src>
			OpCode_BitwiseNot,			// <dst> not  <src>
			OpCode_Negate,				// <dst> neg  <src>

			OpCode_VecExtract,			// <dst> v_extr <src> <imm>
			OpCode_VecSetX,				// <dst> v_setx <src> <src/imm>
			OpCode_VecSetY,				// <dst> v_sety <src> <src/imm>
			OpCode_VecSetZ,				// <dst> v_setz <src> <src/imm>
			OpCode_VecSetW,				// <dst> v_setw <src> <src/imm>

			OpCode_BroadcastVec,		// <dst> vec_bcast <src>

			OpCode_VecSwizzle,			// <dst> vec_swiz <src> <imm>

			OpCode_VecAdd,				// <dst> vec_add <src> <src/imm>
			OpCode_VecSubtract,			// <dst> vec_sub <src> <src/imm>
			OpCode_VecMultiply,			// <dst> vec_mul <src> <src/imm>
			OpCode_VecDivide,			// <dst> vec_div <src> <src/imm>

			OpCode_VecFusedMultiplyAdd,	// <dst> vec_fma <src> <src/imm>

			OpCode_VecDot,				// <dst> vec_dot <src> <src>
			OpCode_VecCross,			// <dst> vec_crs <src> <src>

			OpCode_VecSaturate,			// <dst> vec_sat <src>

			OpCode_VecClamp,			// <dst> vec_clamp <src> <src> <src>
			OpCode_VecLerp				// <dst> vec_lerp  <src> <src> <src/imm>
		};

		inline static bool IsBasic(ILOpCode opcode)
		{
			switch (opcode)
			{
			case OpCode_Noop:
			case OpCode_Discard:
				return true;
			default:
				return false;
			}
		}

		inline static bool IsUnary(ILOpCode opcode)
		{
			switch (opcode)
			{
			case OpCode_Zero:
			case OpCode_Increment:
			case OpCode_Decrement:
			case OpCode_LogicalNot:
			case OpCode_BitwiseNot:
			case OpCode_Negate:
			case OpCode_Cast:
				return true;
			default:
				return false;
			}
		}

		inline static bool IsBinary(ILOpCode opcode)
		{
			switch (opcode)
			{
			case OpCode_Add:
			case OpCode_Subtract:
			case OpCode_Multiply:
			case OpCode_Divide:
			case OpCode_Modulus:
			case OpCode_BitwiseShiftLeft:
			case OpCode_BitwiseShiftRight:
			case OpCode_AndAnd:
			case OpCode_OrOr:
			case OpCode_BitwiseAnd:
			case OpCode_BitwiseOr:
			case OpCode_BitwiseXor:
			case OpCode_LessThan:
			case OpCode_LessThanOrEqual:
			case OpCode_GreaterThan:
			case OpCode_GreaterThanOrEqual:
			case OpCode_Equal:
			case OpCode_NotEqual:
				return true;
			default:
				return false;
			}
		}

		inline static bool IsLoadStore(ILOpCode opcode)
		{
			switch (opcode)
			{
			case OpCode_Store:
			case OpCode_Load:
			case OpCode_LoadParam:
			case OpCode_StoreParam:
			case OpCode_StoreRefParam:
			case OpCode_LoadRefParam:
				return true;
			default:
				return false;
			}
		}

		inline static bool IsDestination(ILOpCode opcode)
		{
			return IsUnary(opcode) || IsBinary(opcode) || IsLoadStore(opcode) || opcode == OpCode_Return || opcode == OpCode_Call || opcode == OpCode_Phi;
		}

		inline static bool IsJump(ILOpCode opcode)
		{
			switch (opcode)
			{
			case OpCode_Jump:
			case OpCode_JumpZero:
			case OpCode_JumpNotZero:
				return true;
			default:
				return false;
			}
		}

		inline static bool IsCall(ILOpCode opcode)
		{
			return opcode == OpCode_Call;
		}

		inline static bool IsReturn(ILOpCode opcode)
		{
			return opcode == OpCode_Return;
		}

		inline static bool IsPhi(ILOpCode opcode)
		{
			return opcode == OpCode_Phi;
		}

		static bool IsCommutative(ILOpCode code)
		{
			switch (code)
			{
				// Arithmetic operations
			case OpCode_Add:
			case OpCode_Multiply:

				// Logical operations
			case OpCode_AndAnd:
			case OpCode_OrOr:

				// Bitwise operations
			case OpCode_BitwiseAnd:
			case OpCode_BitwiseOr:
			case OpCode_BitwiseXor:

				// Comparison operations
			case OpCode_Equal:
			case OpCode_NotEqual:

				// Vector operations
			case OpCode_VecAdd:
			case OpCode_VecMultiply:
				return true;

			default:
				return false;
			}
		}

		struct BasicBlock;

		class Instruction : public IntrusiveLinkedBase<Instruction>, public Value
		{
		public:
			using operands_vector = static_vector<Operand*, StdBumpAllocator<Operand*>>;
		protected:
			BasicBlock* parent = nullptr;
			TextSpan* location = nullptr;
			ILOpCode opcode;
			operands_vector operands;
			Instruction(BumpAllocator& alloc, Value_T id, ILOpCode opcode) : Value(id), opcode(opcode), operands({ alloc }) {}
		public:
			static constexpr Value_T ID = InstructionVal;
			TextSpan* GetLocation() const noexcept { return location; }
			void SetLocation(TextSpan* value) noexcept { location = value; }

			ILOpCode GetOpCode() const noexcept { return opcode; }
			bool IsOp(ILOpCode code) const noexcept { return opcode == code; }

			Operand* GetOperand(size_t idx) const { return operands[idx]; }
			Operand*& GetOperand(size_t idx) { return operands[idx]; }
			const operands_vector& GetOperands() const { return operands; }
			operands_vector& GetOperands() { return operands; }
			size_t OperandCount() const noexcept { return operands.size(); }

			BasicBlock* GetParent() const noexcept { return parent; }
			void SetParent(BasicBlock* newParent) noexcept { parent = newParent; }

			void Dump() const;
			uint64_t hash() const;
			bool operator==(const Instruction& other) const;
			bool operator!=(const Instruction& other) const { return !(*this == other); }
		};

		class BasicInstr : public Instruction
		{
		public:
			static constexpr Value_T ID = BasicInstrVal;
			BasicInstr(BumpAllocator& alloc, ILOpCode opcode) : Instruction(alloc, ID, opcode) {}
		};

		class ResultInstr : public Instruction
		{
		protected:
			ILVarId dst;
			ResultInstr(BumpAllocator& alloc, Value_T id, ILOpCode opcode, const ILVarId& dst) : Instruction(alloc, id, opcode), dst(dst)
			{
			}

		public:
			static constexpr Value_T ID = ResultInstrVal;
			ILVarId& GetResult() noexcept { return dst; }
			const ILVarId& GetResult() const noexcept { return dst; }
			void SetResult(const ILVarId& newRes) noexcept { dst = newRes; }
		};

		class StackAllocInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = StackAllocInstrVal;
			StackAllocInstr(BumpAllocator& alloc, const ILVarId& dst, TypeValue* typeId) : ResultInstr(alloc, ID, OpCode_StackAlloc, dst)
			{
				operands.assign(typeId);
			}
		};

		class OffsetInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = OffsetInstrVal;
			OffsetInstr(BumpAllocator& alloc, const ILVarId& dst, Variable* src, FieldAccess* access) : ResultInstr(alloc, ID, OpCode_OffsetAddress, dst)
			{
				operands.assign(src, access);
			}
		};

		class ReturnInstr : public Instruction
		{
		public:
			static constexpr Value_T ID = ReturnInstrVal;
			ReturnInstr(BumpAllocator& alloc, Operand* target) : Instruction(alloc, ID, OpCode_Return)
			{
				operands.assign(target);
			}
			ReturnInstr(BumpAllocator& alloc) : Instruction(alloc, ID, OpCode_Return)
			{
				operands.assign(nullptr);
			}
		};

		class CallInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = CallInstrVal;
			CallInstr(BumpAllocator& alloc, const ILVarId& dst, Operand* function) : ResultInstr(alloc, ID, OpCode_Call, dst)
			{
				operands.assign(function);
			}

			CallInstr(BumpAllocator& alloc, Operand* function) : ResultInstr(alloc, ID, OpCode_Call, INVALID_VARIABLE)
			{
				operands.assign(function);
			}
		};

		class JumpInstr : public Instruction
		{
		public:
			static constexpr Value_T ID = JumpInstrVal;
			JumpInstr(BumpAllocator& alloc, ILOpCode opcode, Label* target) : Instruction(alloc, ID, opcode)
			{
				operands.assign(target);
			}

			Label* GetLabel() const { return cast<Label>(operands[0]); }
		};

		class BinaryInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = BinaryInstrVal;
			BinaryInstr(BumpAllocator& alloc, ILOpCode opcode, const ILVarId& dst, Operand* lhs, Operand* rhs) : ResultInstr(alloc, ID, opcode, dst)
			{
				operands.assign(lhs, rhs);
			}

			void OverwriteOpCode(ILOpCode value) { opcode = value; }

			Operand* GetLHS() const { return operands[0]; }
			Operand* GetRHS() const { return operands[1]; }

			Operand*& GetLHS() { return operands[0]; }
			Operand*& GetRHS() { return operands[1]; }

			bool IsImmVar() const noexcept { return isa<Constant>(GetLHS()) && isa<Variable>(GetRHS()); }
			bool IsVarImm() const noexcept { return isa<Variable>(GetLHS()) && isa<Constant>(GetRHS()); }
			bool IsImmVar(ILOpCode code) const noexcept { return opcode == code && IsImmVar(); }
			bool IsVarImm(ILOpCode code) const noexcept { return opcode == code && IsVarImm(); }
		};

		class UnaryInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = UnaryInstrVal;
			UnaryInstr(BumpAllocator& alloc, ILOpCode opcode, const ILVarId& dst, Operand* op) : ResultInstr(alloc, ID, opcode, dst)
			{
				operands.assign(op);
			}

			Operand* GetOperand() const { return operands[0]; }
		};

		class StoreInstr : public Instruction
		{
		public:
			static constexpr Value_T ID = StoreInstrVal;
			StoreInstr(BumpAllocator& alloc, Operand* dst, Operand* src) : Instruction(alloc, ID, OpCode_Store)
			{
				operands.assign(dst, src);
			}
		};

		class LoadInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = LoadInstrVal;
			LoadInstr(BumpAllocator& alloc, const ILVarId& dst, Operand* src) : ResultInstr(alloc, ID, OpCode_Load, dst)
			{
				operands.assign(src);
			}
		};

		class StoreParamInstr : public Instruction
		{
		public:
			static constexpr Value_T ID = StoreParamInstrVal;
			StoreParamInstr(BumpAllocator& alloc, Operand* dst, Operand* src) : Instruction(alloc, ID, OpCode_StoreParam)
			{
				operands.assign(dst, src);
			}
		};

		class LoadParamInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = LoadParamInstrVal;
			LoadParamInstr(BumpAllocator& alloc, const ILVarId& dst, Operand* src) : ResultInstr(alloc, ID, OpCode_LoadParam, dst)
			{
				operands.assign(src);
			}
		};

		class MoveInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = MoveInstrVal;
			MoveInstr(BumpAllocator& alloc, const ILVarId& dst, Operand* src) : ResultInstr(alloc, ID, OpCode_Move, dst)
			{
				operands.assign(src);
			}

			Operand* GetSource() { return operands[0]; }
		};

		class PhiInstr : public ResultInstr
		{
		public:
			static constexpr Value_T ID = PhiInstrVal;
			PhiInstr(BumpAllocator& alloc, const ILVarId& dst, size_t operandCount) : ResultInstr(alloc, ID, OpCode_Phi, dst)
			{
				operands.resize(operandCount);
			}
		};

		struct InstructionPtrHash
		{
			std::size_t operator()(const Instruction* ptr) const
			{
				if (!ptr) return 0;
				return ptr->hash();
			}
		};

		struct InstructionPtrEquals
		{
			std::size_t operator()(const Instruction* lhs, const Instruction* rhs) const
			{
				if (lhs == nullptr || rhs == nullptr)
					return lhs == rhs;
				return *lhs == *rhs;
			}
		};
	}
}

#endif