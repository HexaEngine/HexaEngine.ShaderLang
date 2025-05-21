#ifndef IL_INSTRUCTION_HPP
#define IL_INSTRUCTION_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/text_span.hpp"
#include "lexical/numbers.hpp"

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

	static std::string OpCodeToString(ILOpCode opcode)
	{
		switch (opcode)
		{
		case OpCode_Noop: return "nop";
		case OpCode_Alloc: return "alloca";
		case OpCode_Zero: return "zero";
		case OpCode_Store: return "sta";
		case OpCode_Load: return "lda";
		case OpCode_LoadParam: return "ldarg";
		case OpCode_OffsetAddress: return "offs";
		case OpCode_AddressOf: return "addr";
		case OpCode_Push: return "push";
		case OpCode_Pop: return "pop";
		case OpCode_Move: return "mov";
		case OpCode_Cast: return "cast";
		case OpCode_Return: return "ret";
		case OpCode_Jump: return "jmp";
		case OpCode_JumpZero: return "jz";
		case OpCode_JumpNotZero: return "jnz";
		case OpCode_CallBegin: return "cbeg";
		case OpCode_CallEnd: return "cend";
		case OpCode_StoreParam: return "starg";
		case OpCode_Call: return "call";

		case OpCode_Phi: return "phi";

		case OpCode_Add: return "add";
		case OpCode_Subtract: return "sub";
		case OpCode_Multiply: return "mul";
		case OpCode_Divide: return "div";
		case OpCode_Modulus: return "rem";
		case OpCode_BitwiseShiftLeft: return "bls";
		case OpCode_BitwiseShiftRight: return "brs";
		case OpCode_AndAnd: return "lgAnd";
		case OpCode_OrOr: return "lgOr";
		case OpCode_BitwiseAnd: return "bwAnd";
		case OpCode_BitwiseOr: return "bwOr";
		case OpCode_BitwiseXor: return "bwXor";
		case OpCode_LessThan: return "lt";
		case OpCode_LessThanOrEqual: return "ltq";
		case OpCode_GreaterThan: return "gt";
		case OpCode_GreaterThanOrEqual: return "gtq";
		case OpCode_Equal: return "eq";
		case OpCode_NotEqual: return "neq";
		case OpCode_Increment: return "inc";
		case OpCode_Decrement: return "dec";
		case OpCode_LogicalNot: return "lgNot";
		case OpCode_BitwiseNot: return "bwNot";
		case OpCode_Negate: return "neg";

		case OpCode_Vec2Load: return "vec2_lda";
		case OpCode_Vec2Store: return "vec2_sta";
		case OpCode_Vec3Load: return "vec3_lda";
		case OpCode_Vec3Store: return "vec3_sta";
		case OpCode_Vec4Load: return "vec4_lda";
		case OpCode_Vec4Store: return "vec4_sta";

		case OpCode_VecExtract: return "v.extract";

		case OpCode_BroadcastVec2: return "vec2_bcast";
		case OpCode_BroadcastVec3: return "vec3_bcast";
		case OpCode_BroadcastVec4: return "vec4_bcast";

		case OpCode_Vec2Swizzle: return "vec2_swiz";
		case OpCode_Vec3Swizzle: return "vec3_swiz";
		case OpCode_Vec4Swizzle: return "vec4_swiz";

		case OpCode_Vec2Add: return "vec2_add";
		case OpCode_Vec2Subtract: return "vec2_sub";
		case OpCode_Vec2Multiply: return "vec2_mul";
		case OpCode_Vec2Divide: return "vec2_div";

		case OpCode_Vec3Add: return "vec3_add";
		case OpCode_Vec3Subtract: return "vec3_sub";
		case OpCode_Vec3Multiply: return "vec3_mul";
		case OpCode_Vec3Divide: return "vec3_div";

		case OpCode_Vec4Add: return "vec3_add";
		case OpCode_Vec4Subtract: return "vec3_sub";
		case OpCode_Vec4Multiply: return "vec3_mul";
		case OpCode_Vec4Divide: return "vec3_div";

		default: return "Unknown OpCode";
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

	static std::string OperandKindToString(ILOperandKind kind)
	{
		switch (kind)
		{
		case ILOperandKind_Register: return "Register";
		case ILOperandKind_Immediate: return "Immediate";
		case ILOperandKind_Variable: return "Variable";
		case ILOperandKind_Field: return "Field";
		case ILOperandKind_Array: return "Array";
		case ILOperandKind_Label: return "Label";
		case ILOperandKind_Type: return "Type";
		case ILOperandKind_Func: return "Func";
		default: return "Unknown";
		}
	}

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

	static bool operator==(const ILRegister& a, const ILRegister& b)
	{
		return a.id == b.id;
	}

	static bool operator!=(const ILRegister& a, const ILRegister& b)
	{
		return a.id != b.id;
	}

	static bool operator>(const ILRegister& a, const ILRegister& b)
	{
		return a.id > b.id;
	}

	static bool operator<(const ILRegister& a, const ILRegister& b)
	{
		return a.id < b.id;
	}

	static ILRegister operator++(ILRegister& a)
	{
		return a.id++;
	}

	static ILRegister operator--(ILRegister& a)
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

		ILOperand() : kind(ILOperandKind_Disabled) {}

		bool IsDisabled() const noexcept { return kind == ILOperandKind_Disabled; }

		bool IsReg() const noexcept { return kind == ILOperandKind_Register; }

		bool IsVar() const noexcept { return kind == ILOperandKind_Variable; }

		bool IsImm() const noexcept { return kind == ILOperandKind_Immediate; }

		bool IsLabel() const noexcept { return kind == ILOperandKind_Label; }
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
		case ILOperandKind_Label:
			return a.varId == b.varId;

		case ILOperandKind_Field:
			return a.field.typeId == b.field.typeId && a.field.fieldId == b.field.fieldId;

		case ILOperandKind_Disabled:
			return true; // Both are disabled

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

	inline static ILOpKind operator~(ILOpKind value) {
		return (ILOpKind)~(uint32_t)value;
	} inline static ILOpKind operator|(ILOpKind lhs, ILOpKind rhs) {
		return (ILOpKind)((uint32_t)lhs | (uint32_t)rhs);
	} inline static ILOpKind operator&(ILOpKind lhs, ILOpKind rhs) {
		return (ILOpKind)((uint32_t)lhs & (uint32_t)rhs);
	} inline static ILOpKind operator^(ILOpKind lhs, ILOpKind rhs) {
		return (ILOpKind)((uint32_t)lhs ^ (uint32_t)rhs);
	} inline static ILOpKind& operator|=(ILOpKind& lhs, ILOpKind rhs) {
		return (ILOpKind&)((uint32_t&)lhs |= (uint32_t)rhs);
	} inline static ILOpKind& operator&=(ILOpKind& lhs, ILOpKind rhs) {
		return (ILOpKind&)((uint32_t&)lhs &= (uint32_t)rhs);
	} inline static ILOpKind& operator^=(ILOpKind& lhs, ILOpKind rhs) {
		return (ILOpKind&)((uint32_t&)lhs ^= (uint32_t)rhs);
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
	};

	static std::string OpKindToString(ILOpKind opKind)
	{
		switch (opKind)
		{
		case ILOpKind_None: return "";
		case ILOpKind_I8: return "i8";
		case ILOpKind_I16: return "i16";
		case ILOpKind_I32: return "i32";
		case ILOpKind_I64: return "i64";
		case ILOpKind_U8: return "u8";
		case ILOpKind_U16: return "u16";
		case ILOpKind_U32: return "u32";
		case ILOpKind_U64: return "u64";
		case ILOpKind_Half: return "f16";
		case ILOpKind_Float: return "f32";
		case ILOpKind_Double: return "f64";
		case ILOpKind_Min8Float: return "m8f";
		case ILOpKind_Min10Float: return "m10f";
		case ILOpKind_Min16Float: return "m16f";
		case ILOpKind_Min12Int: return "m12i";
		case ILOpKind_Min16Int: return "m16i";
		case ILOpKind_Min16Uint: return "m16u";
		default: return "Unknown OpKind";
		}
	}

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
}

#endif