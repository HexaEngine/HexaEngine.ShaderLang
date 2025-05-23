#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "lexical/numbers.hpp"
#include "utils/hashing.hpp"

namespace HXSL
{
	enum ILOpCode
	{
		OpCode_Noop,
		OpCode_Alloc,
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

	enum ILOperandKind : char
	{
		ILOperandKind_Disabled,
		ILOperandKind_Register,
		ILOperandKind_Immediate,
		ILOperandKind_Variable,
		ILOperandKind_Field,
		ILOperandKind_Array,
		ILOperandKind_Label,
		ILOperandKind_Type,
		ILOperandKind_Func,
		ILOperandKind_Phi,
	};

	struct ILFieldAccess
	{
		uint32_t typeId;
		uint32_t fieldId;
	};

	struct ILRegister
	{
		uint64_t id;

		ILRegister() : id(-1) {}

		constexpr ILRegister(const uint64_t& id) : id(id)
		{
		}
	};

	inline static bool operator==(const ILRegister& a, const ILRegister& b)
	{
		return a.id == b.id;
	}

	inline static bool operator!=(const ILRegister& a, const ILRegister& b)
	{
		return a.id != b.id;
	}

	inline static bool operator>(const ILRegister& a, const ILRegister& b)
	{
		return a.id > b.id;
	}

	inline static bool operator<(const ILRegister& a, const ILRegister& b)
	{
		return a.id < b.id;
	}

	inline static ILRegister operator++(ILRegister& a)
	{
		return a.id++;
	}

	inline static ILRegister operator--(ILRegister& a)
	{
		return a.id--;
	}

	constexpr ILRegister MAX_REGISTERS = 8;

	constexpr ILRegister INVALID_REGISTER = -1;

	struct ILOperand
	{
		ILOperandKind kind;
		union
		{
			ILRegister reg;
			Number imm;
			uint64_t varId;
			ILFieldAccess field;
		};

		ILOperand(ILRegister r) : kind(ILOperandKind_Register), reg(r) {}

		ILOperand(Number i) : kind(ILOperandKind_Immediate), imm(i) {}

		ILOperand(ILOperandKind k, uint64_t v) : kind(k), varId(v) {}

		ILOperand(ILFieldAccess f) : kind(ILOperandKind_Field), field(f) {}

		ILOperand() : kind(ILOperandKind_Disabled), imm() {}

		bool IsDisabled() const noexcept { return kind == ILOperandKind_Disabled; }

		bool IsReg() const noexcept { return kind == ILOperandKind_Register; }

		bool IsVar() const noexcept { return kind == ILOperandKind_Variable; }

		bool IsImm() const noexcept { return kind == ILOperandKind_Immediate; }

		bool IsLabel() const noexcept { return kind == ILOperandKind_Label; }

		uint64_t hash() const noexcept
		{
			XXHash3_64 hash{};
			hash.Combine(kind);
			switch (kind)
			{
			case ILOperandKind_Register:
				hash.Combine(reg.id);
				break;
			case ILOperandKind_Immediate:
				hash.Combine(imm.hash());
				break;
			case ILOperandKind_Variable:
			case ILOperandKind_Type:
			case ILOperandKind_Label:
			case ILOperandKind_Func:
			case ILOperandKind_Phi:
				hash.Combine(varId);
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

		switch (a.kind)
		{
		case ILOperandKind_Register:
			return a.reg == b.reg;

		case ILOperandKind_Immediate:
			return a.imm == b.imm;

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

	constexpr uint32_t ILOpKindTypeMask = ~((1 << ILOpKindFlagBits) - 1);

	enum ILOpKind : uint32_t
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
		return (ILOpKind)~(uint32_t)value;
	}

	inline static ILOpKind operator|(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint32_t)lhs | (uint32_t)rhs);
	}

	inline static ILOpKind operator&(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint32_t)lhs & (uint32_t)rhs);
	}

	inline static ILOpKind operator^(ILOpKind lhs, ILOpKind rhs)
	{
		return (ILOpKind)((uint32_t)lhs ^ (uint32_t)rhs);
	}

	inline static ILOpKind& operator|=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint32_t&)lhs |= (uint32_t)rhs);
	}

	inline static ILOpKind& operator&=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint32_t&)lhs &= (uint32_t)rhs);
	}

	inline static ILOpKind& operator^=(ILOpKind& lhs, ILOpKind rhs)
	{
		return (ILOpKind&)((uint32_t&)lhs ^= (uint32_t)rhs);
	}

	static bool IsFlagSet(ILOpKind opKind, ILOpKind flag)
	{
		return (opKind & flag) != 0;
	}

	struct ILInstruction
	{
		ILOpCode opcode;
		ILOperand operandLeft;
		ILOperand operandRight;
		ILOperand operandResult;
		ILOpKind opKind;

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

		bool IsImmReg() const noexcept { return operandLeft.IsImm() && operandRight.IsReg(); }

		bool IsRegImm() const noexcept { return operandLeft.IsReg() && operandRight.IsImm(); }

		bool IsImmReg(ILOpCode code) const noexcept { return opcode == code && IsImmReg(); }

		bool IsRegImm(ILOpCode code) const noexcept { return opcode == code && IsRegImm(); }

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
	struct hash<HXSL::ILRegister>
	{
		size_t operator()(const HXSL::ILRegister& reg) const noexcept
		{
			return hash<uint64_t>{}(reg.id);
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