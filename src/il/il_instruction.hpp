#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "lexical/numbers.hpp"
#include "utils/hashing.hpp"

namespace HXSL
{
	enum ILOpCode : uint16_t
	{
		OpCode_Noop,
		OpCode_StackAlloc,
		OpCode_Zero,
		OpCode_Store,
		OpCode_Load,
		OpCode_LoadParam,
		OpCode_OffsetAddress,
		OpCode_AddressOf,
		OpCode_Push,
		OpCode_Pop,
		OpCode_Move,

		OpCode_Return,
		OpCode_CallBegin,
		OpCode_CallEnd,
		OpCode_StoreParam,
		OpCode_Call,

		OpCode_Jump,
		OpCode_JumpZero,
		OpCode_JumpNotZero,

		OpCode_Cast,

		OpCode_Discard,

		OpCode_Phi,

		OpCode_Add,
		OpCode_Subtract,
		OpCode_Multiply,
		OpCode_Divide,
		OpCode_Modulus,
		OpCode_BitwiseShiftLeft,
		OpCode_BitwiseShiftRight,
		OpCode_AndAnd,
		OpCode_OrOr,
		OpCode_BitwiseAnd,
		OpCode_BitwiseOr,
		OpCode_BitwiseXor,
		OpCode_LessThan,
		OpCode_LessThanOrEqual,
		OpCode_GreaterThan,
		OpCode_GreaterThanOrEqual,
		OpCode_Equal,
		OpCode_NotEqual,
		OpCode_Increment,
		OpCode_Decrement,
		OpCode_LogicalNot,
		OpCode_BitwiseNot,
		OpCode_Negate,

		OpCode_Vec2Load,
		OpCode_Vec2Store,

		OpCode_Vec3Load,
		OpCode_Vec3Store,

		OpCode_Vec4Load,
		OpCode_Vec4Store,

		OpCode_VecExtract,

		OpCode_BroadcastVec2,
		OpCode_BroadcastVec3,
		OpCode_BroadcastVec4,

		OpCode_Vec2Swizzle,
		OpCode_Vec3Swizzle,
		OpCode_Vec4Swizzle,

		OpCode_Vec2Add,
		OpCode_Vec2Subtract,
		OpCode_Vec2Multiply,
		OpCode_Vec2Divide,

		OpCode_Vec3Add,
		OpCode_Vec3Subtract,
		OpCode_Vec3Multiply,
		OpCode_Vec3Divide,

		OpCode_Vec4Add,
		OpCode_Vec4Subtract,
		OpCode_Vec4Multiply,
		OpCode_Vec4Divide,
	};

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
		case OpCode_Vec2Add:
		case OpCode_Vec2Multiply:
		case OpCode_Vec3Add:
		case OpCode_Vec3Multiply:
		case OpCode_Vec4Add:
		case OpCode_Vec4Multiply:
			return true;

		default:
			return false;
		}
	}

	enum ILOperandKind_T : char
	{
		ILOperandKind_Disabled,
		ILOperandKind_Variable,
		ILOperandKind_Field,
		ILOperandKind_Label,
		ILOperandKind_Type,
		ILOperandKind_Func,
		ILOperandKind_Imm_i8,
		ILOperandKind_Imm_u8,
		ILOperandKind_Imm_i16,
		ILOperandKind_Imm_u16,
		ILOperandKind_Imm_i32,
		ILOperandKind_Imm_u32,
		ILOperandKind_Imm_i64,
		ILOperandKind_Imm_u64,
		ILOperandKind_Imm_f16,
		ILOperandKind_Imm_f32,
		ILOperandKind_Imm_f64,
		ILOperandKind_Phi,
	};

	struct ILOperandKind
	{
		ILOperandKind_T value;

		constexpr ILOperandKind(ILOperandKind_T value) : value(value) {}
		constexpr ILOperandKind(NumberType val) : value(val == NumberType_Unknown ? ILOperandKind_Disabled : static_cast<ILOperandKind_T>((val - 1 + ILOperandKind_Imm_i8))) {}
		constexpr operator ILOperandKind_T() const { return value; }
		constexpr operator char() const { return static_cast<char>(value); }
		constexpr operator NumberType() const { return value < ILOperandKind_Imm_i8 || value > ILOperandKind_Imm_f64 ? NumberType_Unknown : static_cast<NumberType>(value - ILOperandKind_Imm_i8 + 1); }

		constexpr bool operator==(const ILOperandKind& other) const { return value == other.value; }
		constexpr bool operator!=(const ILOperandKind& other) const { return value != other.value; }
		constexpr bool operator<(const ILOperandKind& other) const { return value < other.value; }
		constexpr bool operator>(const ILOperandKind& other) const { return value > other.value; }
		constexpr ILOperandKind operator~() const { return static_cast<ILOperandKind_T>(~static_cast<char>(value)); }
		constexpr ILOperandKind operator|(const ILOperandKind& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) | static_cast<char>(other.value)); }
		constexpr ILOperandKind operator&(const ILOperandKind& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) & static_cast<char>(other.value)); }
		constexpr ILOperandKind operator^(const ILOperandKind& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) ^ static_cast<char>(other.value)); }
		constexpr ILOperandKind operator+(const ILOperandKind& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) + static_cast<char>(other.value)); }
		constexpr ILOperandKind operator-(const ILOperandKind& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) - static_cast<char>(other.value)); }

		constexpr bool operator==(const ILOperandKind_T& other) const { return value == other; }
		constexpr bool operator!=(const ILOperandKind_T& other) const { return value != other; }
		constexpr bool operator<(const ILOperandKind_T& other) const { return value > other; }
		constexpr bool operator>(const ILOperandKind_T& other) const { return value < other; }
		constexpr ILOperandKind operator|(const ILOperandKind_T& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) | static_cast<char>(other)); }
		constexpr ILOperandKind operator&(const ILOperandKind_T& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) & static_cast<char>(other)); }
		constexpr ILOperandKind operator^(const ILOperandKind_T& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) ^ static_cast<char>(other)); }
		constexpr ILOperandKind operator+(const ILOperandKind_T& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) + static_cast<char>(other)); }
		constexpr ILOperandKind operator-(const ILOperandKind_T& other) const { return static_cast<ILOperandKind_T>(static_cast<char>(value) - static_cast<char>(other)); }
	};

	struct ILFieldAccess
	{
		uint32_t typeId;
		uint32_t fieldId;
	};

	using ILTypeId = uint64_t;
	using ILFuncId = uint64_t;
	using ILLabel = uint64_t;
	using ILPhiId = uint64_t;

	struct ILVarId_T
	{
		uint64_t id : 32;
		uint64_t version : 31;
		uint64_t temp : 1;
	};

	struct ILVarId
	{
		union
		{
			ILVarId_T var;
			uint64_t raw;
		};

		constexpr ILVarId(uint64_t raw) : raw(raw) {}
		constexpr ILVarId(ILVarId_T var) : var(var) {}
		constexpr ILVarId() : raw(0) {}

		constexpr operator uint64_t() const { return raw; }
		constexpr operator ILVarId_T() const { return var; }

		constexpr bool operator==(const ILVarId& other) const { return raw == other.raw; }
		constexpr bool operator!=(const ILVarId& other) const { return raw != other.raw; }

		constexpr ILVarId& operator++() { ++raw; return *this; }
		constexpr ILVarId& operator--() { --raw; return *this; }
		constexpr ILVarId operator++(int) { auto tmp = *this; ++raw; return tmp; }
		constexpr ILVarId operator--(int) { auto tmp = *this; --raw; return tmp; }
	};

	constexpr ILVarId INVALID_VARIABLE = -1;

	struct ILOperand
	{
		union
		{
			NumberUnion imm_m;
			ILVarId varId;
			ILTypeId typeId;
			ILFuncId funcId;
			ILLabel label;
			ILFieldAccess field;
			ILPhiId phiId;
			uint64_t raw;
		};

		ILOperandKind kind;

		constexpr ILOperand(ILVarId v) : kind(ILOperandKind_Variable), varId(v) {}

		ILOperand(Number i) : kind(i.Kind), imm_m(i) {}

		constexpr ILOperand(ILOperandKind k, const uint64_t& v) : kind(k), raw(v) {}

		constexpr ILOperand(ILFieldAccess f) : kind(ILOperandKind_Field), field(f) {}

		ILOperand() : kind(ILOperandKind_Disabled), imm_m() {}

		bool IsDisabled() const noexcept { return kind == ILOperandKind_Disabled; }

		bool IsVar() const noexcept { return kind == ILOperandKind_Variable; }

		bool IsImm() const noexcept { return kind >= ILOperandKind_Imm_i8 && kind <= ILOperandKind_Imm_f64; }

		bool IsLabel() const noexcept { return kind == ILOperandKind_Label; }

		Number imm() const noexcept { return Number(imm_m, kind); }

		uint64_t hash() const noexcept
		{
			XXHash3_64 hash{};
			hash.Combine(kind.value);
			switch (kind.value)
			{
			case ILOperandKind_Imm_i8:
			case ILOperandKind_Imm_u8:
			case ILOperandKind_Imm_i16:
			case ILOperandKind_Imm_u16:
			case ILOperandKind_Imm_i32:
			case ILOperandKind_Imm_u32:
			case ILOperandKind_Imm_i64:
			case ILOperandKind_Imm_u64:
			case ILOperandKind_Imm_f16:
			case ILOperandKind_Imm_f32:
			case ILOperandKind_Imm_f64:
				hash.Combine(imm().hash());
				break;
			case ILOperandKind_Variable:
			case ILOperandKind_Type:
			case ILOperandKind_Label:
			case ILOperandKind_Func:
			case ILOperandKind_Phi:
				hash.Combine(varId.raw);
				break;
			case ILOperandKind_Field:
				hash.Combine(field.typeId);
				hash.Combine(field.fieldId);
				break;
			}

			return hash.Finalize();
		}
	};

	static bool operator==(const ILOperand& a, const ILOperand& b)
	{
		if (a.kind != b.kind)
			return false;

		switch (a.kind.value)
		{
		case ILOperandKind_Imm_i8:
		case ILOperandKind_Imm_u8:
		case ILOperandKind_Imm_i16:
		case ILOperandKind_Imm_u16:
		case ILOperandKind_Imm_i32:
		case ILOperandKind_Imm_u32:
		case ILOperandKind_Imm_i64:
		case ILOperandKind_Imm_u64:
		case ILOperandKind_Imm_f16:
		case ILOperandKind_Imm_f32:
		case ILOperandKind_Imm_f64:
			return a.imm() == b.imm();

		case ILOperandKind_Variable:
		case ILOperandKind_Type:
		case ILOperandKind_Label:
		case ILOperandKind_Func:
		case ILOperandKind_Phi:
			return a.varId == b.varId;

		case ILOperandKind_Field:
			return a.field.typeId == b.field.typeId && a.field.fieldId == b.field.fieldId;

		case ILOperandKind_Disabled:
			return true;

		default:
			return false;
		}
	}

	static bool operator!=(const ILOperand& a, const ILOperand& b)
	{
		return !(a == b);
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

	struct ILInstruction // alignment 8
	{
		ILOpCode opcode; // 2 bytes
		ILOpKind opKind; // 1 byte
		// padding 5 bytes.
		ILOperand operandLeft; // 16 bytes
		ILOperand operandRight; // 16 bytes
		ILOperand operandResult; // 16 bytes

		ILInstruction(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandRight, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), operandRight(operandRight), operandResult(operandResult), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, const ILOperand& operandLeft, const ILOperand& operandResult, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), operandResult(operandResult), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, const ILOperand& operandLeft, ILOpKind opKind = ILOpKind_None) : opcode(opcode), operandLeft(operandLeft), opKind(opKind)
		{
		}

		ILInstruction(ILOpCode opcode, ILOpKind opKind = ILOpKind_None) : opcode(opcode), opKind(opKind)
		{
		}

		ILInstruction() : opcode(OpCode_Noop), opKind(ILOpKind_None)
		{
		}

		bool IsOp(ILOpCode code) const noexcept { return opcode == code; }

		bool IsImmVar() const noexcept { return operandLeft.IsImm() && operandRight.IsVar(); }

		bool IsVarImm() const noexcept { return operandLeft.IsVar() && operandRight.IsImm(); }

		bool IsImmVar(ILOpCode code) const noexcept { return opcode == code && IsImmVar(); }

		bool IsVarImm(ILOpCode code) const noexcept { return opcode == code && IsVarImm(); }

		uint64_t hash() const noexcept
		{
			XXHash3_64 hash{};
			hash.Combine(opcode);
			hash.Combine(opKind);

			uint64_t leftHash = operandLeft.hash();
			uint64_t rightHash = operandRight.hash();

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

			if (operandLeft != other.operandLeft || operandRight != other.operandRight)
			{
				if (!IsCommutative(opcode) || operandLeft != other.operandRight || operandRight != other.operandLeft)
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
		uint32_t start;
		uint32_t len;
		TextSpan span;

		ILMapping(const uint32_t& start, const uint32_t& len, const TextSpan& span)
			: start(start), len(len), span(span)
		{
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
	struct hash<HXSL::ILOperand>
	{
		size_t operator()(const HXSL::ILOperand& op) const noexcept
		{
			return static_cast<size_t>(op.hash());
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