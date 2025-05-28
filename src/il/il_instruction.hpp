#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "lexical/numbers.hpp"
#include "utils/hashing.hpp"
#include "utils/ilist.hpp"
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

		OpCode_Return,			// ret <src/imm/dis>

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

	class Instruction : public IntrusiveLinkedBase<Instruction>
	{
	protected:
		TextSpan* location = nullptr;
		ILOpCode opcode;
		Instruction(ILOpCode opcode) : opcode(opcode) {}
	public:
		TextSpan* GetLocation() const noexcept { return location; }
		void SetLocation(TextSpan* value) noexcept { location = value; }

		ILOpCode GetOpCode() const noexcept { return opcode; }
		void SetOpCode(ILOpCode value) noexcept { opcode = value; }

		inline bool IsBasic() const { return HXSL::IsBasic(opcode); }
		inline bool IsUnary() const { return HXSL::IsUnary(opcode); }
		inline bool IsBinary() const { return HXSL::IsBinary(opcode); }
		inline bool IsLoadStore() const { return HXSL::IsLoadStore(opcode); }
		inline bool IsDestination() const { return HXSL::IsDestination(opcode); }
		inline bool IsJump() const { return HXSL::IsJump(opcode); }
		inline bool IsCall() const { return HXSL::IsCall(opcode); }
		inline bool IsReturn() const { return HXSL::IsReturn(opcode); }
		inline bool IsPhi() const { return HXSL::IsPhi(opcode); }
	};

	class BasicInstruction : public Instruction
	{
	public:
		BasicInstruction(ILOpCode opcode) : Instruction(opcode) {}
	};

	class DestinationInstruction : public Instruction
	{
	protected:
		Operand* dst;
		DestinationInstruction(ILOpCode opcode) : Instruction(opcode) {}

	public:
		Operand*& OpDst() noexcept { return dst; }
		Operand* OpDst() const noexcept { return dst; }
	};

	class ReturnInstruction : public DestinationInstruction
	{
	public:
		ReturnInstruction() : DestinationInstruction(OpCode_Return) {}
	};

	class CallInstruction : public DestinationInstruction
	{
		Operand* function;

	public:
		CallInstruction() : DestinationInstruction(OpCode_Call) {}

		Operand*& Function() noexcept { return function; }
		Operand* Function() const noexcept { return function; }
	};

	class JumpInstruction : public Instruction
	{
		Operand* target;

	public:
		JumpInstruction(ILOpCode opcode) : Instruction(opcode) {}

		Operand*& Target() noexcept { return target; }
		Operand* Target() const noexcept { return target; }
	};

	class BinaryInstruction : public DestinationInstruction
	{
		Operand* lhs;
		Operand* rhs;

	public:
		BinaryInstruction(ILOpCode opcode) : DestinationInstruction(opcode) {}
		Operand*& OpLhs() noexcept { return lhs; }
		Operand*& OpRhs() noexcept { return rhs; }
		Operand* OpLhs() const noexcept { return lhs; }
		Operand* OpRhs() const noexcept { return rhs; }
	};

	class UnaryInstruction : public DestinationInstruction
	{
		Operand* op;
	public:
		UnaryInstruction(ILOpCode opcode) : DestinationInstruction(opcode) {}
		Operand*& Op() noexcept { return op; }
		Operand* Op() const noexcept { return op; }
	};

	class LoadStoreInstruction : public DestinationInstruction
	{
		Operand* src;

	public:
		LoadStoreInstruction(ILOpCode opcode) : DestinationInstruction(opcode) {}
		Operand*& OpSrc() noexcept { return src; }
		Operand* OpSrc() const noexcept { return src; }
	};

	class ILInstruction : public IntrusiveLinkedBase<ILInstruction>
	{
	public:
		TextSpan* location = nullptr;
		ILOpCode opcode;
		ILOpKind opKind;
		Operand* operandLeft = nullptr;
		Operand* operandRight = nullptr;
		Operand* operandResult = nullptr;

		ILInstruction(ILOpCode opcode, Operand* operandLeft, Operand* operandRight, Operand* operandResult, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), operandRight(operandRight), operandResult(operandResult), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, Operand* operandLeft, Operand* operandResult, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), operandResult(operandResult), opKind(opKind)
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

			if (equals(operandLeft, other.operandLeft) || equals(operandRight, other.operandRight))
			{
				if (!IsCommutative(opcode) || equals(operandLeft, other.operandRight) || equals(operandRight, other.operandLeft))
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

	struct ILMapping
	{
		ILInstruction* start;
		ILInstruction* end;
		TextSpan span;

		ILMapping(ILInstruction* start, ILInstruction* end, const TextSpan& span)
			: start(start), end(end), span(span)
		{
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
}

namespace std
{
	template <>
	struct hash<HXSL::ILVarId>
	{
		size_t operator()(const HXSL::ILVarId& var) const noexcept
		{
			return hash<uint64_t>{}(var.raw);
		}
	};

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