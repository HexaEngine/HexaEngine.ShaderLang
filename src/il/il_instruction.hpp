#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "lexical/numbers.hpp"
#include "utils/hashing.hpp"
#include "utils/ilist.hpp"
#include "utils/static_vector.hpp"
#include "value.hpp"
#include "operands.hpp"

namespace HXSL
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

		OpCode_Phi,				// <dst> phi <phi_id>

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

	constexpr size_t ILOpKindFlagBits = 2;

	constexpr uint8_t ILOpKindTypeMask = ~((1 << ILOpKindFlagBits) - 1);

	enum ILOpKind : uint8_t
	{
		ILOpKind_None = 0,
		ILOpKind_Const = 1,
		ILOpKind_Precise = 2,
		ILOpKind_I8 = 1 << ILOpKindFlagBits,
		ILOpKind_I16 = 2 << ILOpKindFlagBits,
		ILOpKind_I32 = 3 << ILOpKindFlagBits,
		ILOpKind_I64 = 4 << ILOpKindFlagBits,
		ILOpKind_U8 = 5 << ILOpKindFlagBits,
		ILOpKind_U16 = 6 << ILOpKindFlagBits,
		ILOpKind_U32 = 7 << ILOpKindFlagBits,
		ILOpKind_U64 = 8 << ILOpKindFlagBits,
		ILOpKind_Half = 9 << ILOpKindFlagBits,
		ILOpKind_Float = 10 << ILOpKindFlagBits,
		ILOpKind_Double = 11 << ILOpKindFlagBits,
		ILOpKind_Min8Float = 12 << ILOpKindFlagBits,
		ILOpKind_Min10Float = 13 << ILOpKindFlagBits,
		ILOpKind_Min16Float = 14 << ILOpKindFlagBits,
		ILOpKind_Min12Int = 15 << ILOpKindFlagBits,
		ILOpKind_Min16Int = 16 << ILOpKindFlagBits,
		ILOpKind_Min16Uint = 17 << ILOpKindFlagBits,
	};

	inline static ILOpKind operator~(ILOpKind value)
	{
		return (ILOpKind)~(uint8_t)value;
	}

	inline static ILOpKind operator|(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint8_t)lhs | (uint8_t)rhs);
	}

	inline static ILOpKind operator&(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint8_t)lhs & (uint8_t)rhs);
	}

	inline static ILOpKind operator^(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint8_t)lhs ^ (uint8_t)rhs);
	}

	inline static ILOpKind& operator|=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint8_t&)lhs |= (uint8_t)rhs);
	}

	inline static ILOpKind& operator&=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint8_t&)lhs &= (uint8_t)rhs);
	}

	inline static ILOpKind& operator^=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint8_t&)lhs ^= (uint8_t)rhs);
	}

	static bool IsFlagSet(ILOpKind opKind, ILOpKind flag)
	{
		return (opKind & flag) != 0;
	}

	class ILInstruction : public IntrusiveLinkedBase<ILInstruction>
	{
	public:
		TextSpan* location = nullptr;
		ILOpCode opcode;
		ILOpKind opKind;
		ILVarId result = INVALID_VARIABLE;
		Value* operandLeft = nullptr;
		Value* operandRight = nullptr;

		ILInstruction(ILOpCode opcode, ILVarId result, Value* operandLeft, Value* operandRight, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), operandRight(operandRight), result(result), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, ILVarId result, Value* operandLeft, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), result(result), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, ILVarId result, ILOpKind opKind = ILOpKind_None) : opcode(opcode), result(result), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, Operand* operandLeft, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, ILOpKind opKind = ILOpKind_None) : opcode(opcode), opKind(opKind)
		{
		}

		ILInstruction() : opcode(OpCode_Noop), opKind(ILOpKind_None)
		{
		}

		bool HasResult() const noexcept { return result != INVALID_VARIABLE; }

		bool IsOp(ILOpCode code) const noexcept { return opcode == code; }

		bool IsImmVar() const noexcept { return isa<Constant>(operandLeft) && isa<Variable>(operandRight); }

		bool IsVarImm() const noexcept { return isa<Variable>(operandLeft) && isa<Constant>(operandRight); }

		bool IsImmVar(ILOpCode code) const noexcept { return opcode == code && IsImmVar(); }

		bool IsVarImm(ILOpCode code) const noexcept { return opcode == code && IsVarImm(); }

		uint64_t hash() const noexcept
		{
			XXHash3_64 hash{};
			hash.Combine(opcode);
			hash.Combine(opKind);

			uint64_t leftHash = HXSL::hash(operandLeft);
			uint64_t rightHash = HXSL::hash(operandRight);

			if (IsCommutative(opcode) && leftHash > rightHash)
			{
				std::swap(leftHash, rightHash);
			}

			hash.Combine(leftHash);
			hash.Combine(rightHash);

			return hash.Finalize();
		}

		bool operator==(const ILInstruction& other) const noexcept
		{
			if (opcode != other.opcode || opKind != other.opKind)
			{
				return false;
			}

			if (!equals(operandLeft, other.operandLeft) || !equals(operandRight, other.operandRight))
			{
				if (!IsCommutative(opcode) || !equals(operandLeft, other.operandRight) || !equals(operandRight, other.operandLeft))
				{
					return false;
				}
			}

			return true;
		}

		bool operator!=(const ILInstruction& other)
		{
			return !(*this == other);
		}
	};

	struct ILInstructionPtrHash
	{
		std::size_t operator()(const ILInstruction* ptr) const
		{
			if (!ptr) return 0;
			return ptr->hash();
		}
	};

	struct ILInstructionPtrEquals
	{
		std::size_t operator()(const ILInstruction* lhs, const ILInstruction* rhs) const
		{
			if (lhs == nullptr || rhs == nullptr)
				return lhs == rhs;
			return *lhs == *rhs;
		}
	};

	struct BasicBlock;

	class Instruction : public IntrusiveLinkedBase<Instruction>, public Value
	{
	protected:
		BasicBlock* parent = nullptr;
		TextSpan* location = nullptr;
		ILOpCode opcode;
		static_vector<Operand*, StdBumpAllocator<Operand*>> operands;
		Instruction(BumpAllocator& alloc, Value_T id, ILOpCode opcode) : Value(id), opcode(opcode), operands({ alloc }) {}
	public:
		static constexpr Value_T ID = InstructionVal;
		TextSpan* GetLocation() const noexcept { return location; }
		void SetLocation(TextSpan* value) noexcept { location = value; }

		ILOpCode GetOpCode() const noexcept { return opcode; }

		Operand* GetOperand(size_t idx) const { return operands[idx]; }
		Operand*& GetOperand(size_t idx) { return operands[idx]; }
		size_t OperandCount() const noexcept { return operands.size(); }

		BasicBlock* GetParent() const noexcept { return parent; }
		void SetParent(BasicBlock* newParent) noexcept { parent = newParent; }

		void Dump() const;
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
		ResultInstr(BumpAllocator& alloc, Value_T id, ILOpCode opcode, const ILVarId& dst) : Instruction(alloc, id, opcode), dst(dst) {}

	public:
		static constexpr Value_T ID = ResultInstrVal;
		ILVarId& OpDst() noexcept { return dst; }
		const ILVarId& OpDst() const noexcept { return dst; }
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
	};

	class BinaryInstr : public ResultInstr
	{
	public:
		static constexpr Value_T ID = BinaryInstrVal;
		BinaryInstr(BumpAllocator& alloc, ILOpCode opcode, const ILVarId& dst, Operand* lhs, Operand* rhs) : ResultInstr(alloc, ID, opcode, dst)
		{
			operands.assign(lhs, rhs);
		}
	};

	class UnaryInstr : public ResultInstr
	{
		Operand* op;
	public:
		static constexpr Value_T ID = UnaryInstrVal;
		UnaryInstr(BumpAllocator& alloc, ILOpCode opcode, const ILVarId& dst, Operand* op) : ResultInstr(alloc, ID, opcode, dst), op(op)
		{
			operands.assign(op);
		}
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
	};

	class PhiInstr
	{
	};
}

namespace std
{
	template <>
	struct hash<HXSL::ILInstruction>
	{
		size_t operator()(const HXSL::ILInstruction& instr) const noexcept
		{
			return static_cast<size_t>(instr.hash());
		}
	};
}

#endif