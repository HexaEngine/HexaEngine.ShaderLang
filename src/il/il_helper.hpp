#ifndef IL_HELPER_HPP
#define IL_HELPER_HPP

#include "pch/ast.hpp"
#include "il_instruction.hpp"
#include "il_metadata.hpp"

namespace HXSL
{
	static ILOpCode OperatorToOpCode(Operator op)
	{
		switch (op)
		{
		case Operator_Add:
			return OpCode_Add;
		case Operator_Subtract:
			return OpCode_Subtract;
		case Operator_Multiply:
			return OpCode_Multiply;
		case Operator_Divide:
			return OpCode_Divide;
		case Operator_Modulus:
			return OpCode_Modulus;
		case Operator_BitwiseShiftLeft:
			return OpCode_BitwiseShiftLeft;
		case Operator_BitwiseShiftRight:
			return OpCode_BitwiseShiftRight;
		case Operator_AndAnd:
			return OpCode_AndAnd;
		case Operator_OrOr:
			return OpCode_OrOr;
		case Operator_BitwiseAnd:
			return OpCode_BitwiseAnd;
		case Operator_BitwiseOr:
			return OpCode_BitwiseOr;
		case Operator_BitwiseXor:
			return OpCode_BitwiseXor;
		case Operator_LessThan:
			return OpCode_LessThan;
		case Operator_LessThanOrEqual:
			return OpCode_LessThanOrEqual;
		case Operator_GreaterThan:
			return OpCode_GreaterThan;
		case Operator_GreaterThanOrEqual:
			return OpCode_GreaterThanOrEqual;
		case Operator_Equal:
			return OpCode_Equal;
		case Operator_NotEqual:
			return OpCode_NotEqual;
		case Operator_Increment:
			return OpCode_Increment;
		case Operator_Decrement:
			return OpCode_Decrement;
		case Operator_LogicalNot:
			return OpCode_LogicalNot;
		case Operator_BitwiseNot:
			return OpCode_BitwiseNot;
		default:
			return OpCode_Noop;
		}
	}

	static ILOpCode OperatorToVecOpCode(Operator op, uint32_t components)
	{
		switch (op)
		{
		case Operator_Add:
			return OpCode_VecAdd;
		case Operator_Subtract:
			return OpCode_VecSubtract;
		case Operator_Multiply:
			return OpCode_VecMultiply;
		case Operator_Divide:
			return OpCode_VecDivide;
		}

		return OpCode_Noop;
	}

	static ILOpKind PrimToOpKind(PrimitiveKind kind)
	{
		switch (kind)
		{
		case PrimitiveKind_Bool:
			return ILOpKind_U8;
		case PrimitiveKind_Int:
			return ILOpKind_I32;
		case PrimitiveKind_Float:
			return ILOpKind_Float;
		case PrimitiveKind_UInt:
			return ILOpKind_U32;
		case PrimitiveKind_Double:
			return ILOpKind_Double;
		case PrimitiveKind_Min8Float:
			return ILOpKind_Min8Float;
		case PrimitiveKind_Min10Float:
			return ILOpKind_Min10Float;
		case PrimitiveKind_Min16Float:
			return ILOpKind_Min16Float;
		case PrimitiveKind_Min12Int:
			return ILOpKind_Min12Int;
		case PrimitiveKind_Min16Int:
			return ILOpKind_Min16Int;
		case PrimitiveKind_Min16UInt:
			return ILOpKind_Min16Uint;
		case PrimitiveKind_UInt8:
			return ILOpKind_U8;
		case PrimitiveKind_Int16:
			return ILOpKind_I16;
		case PrimitiveKind_UInt16:
			return ILOpKind_U16;
		case PrimitiveKind_Half:
			return ILOpKind_Half;
		case PrimitiveKind_Int64:
			return ILOpKind_I64;
		case PrimitiveKind_UInt64:
			return ILOpKind_U64;
		default:
			return ILOpKind_None;
		}
	}

	static ILOpKind NumToOpKind(NumberType type)
	{
		switch (type)
		{
		case NumberType_Int8:
			return ILOpKind_I8;
		case NumberType_UInt8:
			return ILOpKind_U8;
		case NumberType_Int16:
			return ILOpKind_I16;
		case NumberType_UInt16:
			return ILOpKind_U16;
		case NumberType_Int32:
			return ILOpKind_I32;
		case NumberType_UInt32:
			return ILOpKind_U32;
		case NumberType_Int64:
			return ILOpKind_I64;
		case NumberType_UInt64:
			return ILOpKind_U64;
		case NumberType_Half:
			return ILOpKind_Half;
		case NumberType_Float:
			return ILOpKind_Float;
		case NumberType_Double:
			return ILOpKind_Double;
		default:
			return ILOpKind_None;
		}
	}

	static ILOpKind PrimToOpKind(OperatorOverload* op)
	{
		Primitive* prim = dynamic_cast<Primitive*>(op->GetReturnType());
		return PrimToOpKind(prim->GetKind());
	}

	static bool IsImmediate(Expression* expr, Number& num)
	{
		if (expr->IsTypeOf(NodeType_LiteralExpression))
		{
			auto literal = expr->As<LiteralExpression>();
			auto& token = literal->GetLiteral();
			if (token.isNumeric())
			{
				num = token.Numeric;
			}
			else if (token.isBool())
			{
				num = Number(token.Value == Keyword_True);
			}
			else
			{
				HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
			}

			return true;
		}
		return false;
	}

	static ILOpCode VecLoadOp(const ILVariable& var, bool addressOf = false)
	{
		if (!var.IsReference()) return OpCode_Move;
		return addressOf ? OpCode_AddressOf : OpCode_Load;
	}

	static ILOpCode VecStoreOp(const ILVariable& var)
	{
		if (!var.IsReference()) return OpCode_Move;
		return OpCode_Store;
	}

#define DEFINE_CAST(field, type) \
  __pragma(warning(push)) \
  __pragma(warning(disable:4244)) \
  switch (numType) { \
    case NumberType_Int8: out.field = static_cast<type>(input.i8); return out; \
    case NumberType_UInt8: out.field = static_cast<type>(input.u8); return out; \
    case NumberType_Int16: out.field = static_cast<type>(input.i16); return out; \
    case NumberType_UInt16: out.field = static_cast<type>(input.u16); return out; \
    case NumberType_Int32: out.field = static_cast<type>(input.i32); return out; \
    case NumberType_UInt32: out.field = static_cast<type>(input.u32); return out; \
    case NumberType_Int64: out.field = static_cast<type>(input.i64); return out; \
    case NumberType_UInt64: out.field = static_cast<type>(input.u64); return out; \
    case NumberType_Half: out.field = static_cast<type>(input.half_); return out; \
    case NumberType_Float: out.field = static_cast<type>(input.float_); return out; \
    case NumberType_Double: out.field = static_cast<type>(input.double_); return out; \
    default: return {}; \
  } \
  __pragma(warning(pop))

	static Number Cast(Number input, ILOpKind kind)
	{
		auto numType = input.Kind;
		auto type = static_cast<ILOpKind>(kind & static_cast<ILOpKind>(ILOpKindTypeMask));
		Number out;

		switch (type)
		{
		case ILOpKind_I8:
			out.Kind = NumberType_Int8;
			DEFINE_CAST(i8, int8_t)
				break;
		case ILOpKind_I16:
			out.Kind = NumberType_Int16;
			DEFINE_CAST(i16, int16_t)
				break;
		case ILOpKind_I32:
			out.Kind = NumberType_Int32;
			DEFINE_CAST(i32, int32_t)
				break;
		case ILOpKind_I64:
			out.Kind = NumberType_Int64;
			DEFINE_CAST(i64, int64_t)
				break;
		case ILOpKind_U8:
			out.Kind = NumberType_UInt8;
			DEFINE_CAST(u8, uint8_t)
				break;
		case ILOpKind_U16:
			out.Kind = NumberType_UInt16;
			DEFINE_CAST(u16, uint16_t)
				break;
		case ILOpKind_U32:
			out.Kind = NumberType_UInt32;
			DEFINE_CAST(u32, uint32_t)
				break;
		case ILOpKind_U64:
			out.Kind = NumberType_UInt64;
			DEFINE_CAST(u64, uint64_t)
				break;
		case ILOpKind_Half:
			out.Kind = NumberType_Half;
			DEFINE_CAST(half_, half)
				break;
		case ILOpKind_Float:
			out.Kind = NumberType_Float;
			DEFINE_CAST(float_, float)
				break;
		case ILOpKind_Double:
			out.Kind = NumberType_Double;
			DEFINE_CAST(double_, double)
				break;
		case ILOpKind_Min8Float:
			break;
		case ILOpKind_Min10Float:
			break;
		case ILOpKind_Min16Float:
			break;
		case ILOpKind_Min12Int:
			break;
		case ILOpKind_Min16Int:
			break;
		case ILOpKind_Min16Uint:
			break;
		default:
			break;
		}
		return {};
	}

	static Number FoldImm(const Number& left, const Number& right, ILOpCode code)
	{
		switch (code)
		{
		case OpCode_Add: return left + right;
		case OpCode_Subtract: return left - right;
		case OpCode_Multiply: return left * right;
		case OpCode_Divide: return left / right;
		case OpCode_Modulus: return left % right;
		case OpCode_BitwiseShiftLeft: return left << right;
		case OpCode_BitwiseShiftRight: return left >> right;
		case OpCode_AndAnd: return Number(left.ToBool() && right.ToBool());
		case OpCode_OrOr: return Number(left.ToBool() || right.ToBool());
		case OpCode_BitwiseAnd: return left & right;
		case OpCode_BitwiseOr: return left | right;
		case OpCode_BitwiseXor: return left ^ right;
		case OpCode_LessThan: return Number(left < right);
		case OpCode_LessThanOrEqual: return Number(left <= right);
		case OpCode_GreaterThan: return Number(left > right);
		case OpCode_GreaterThanOrEqual: return Number(left >= right);
		case OpCode_Equal: return Number(left == right);
		case OpCode_NotEqual: return Number(left != right);
		case OpCode_LogicalNot: return Number(!left.ToBool());
		case OpCode_BitwiseNot: return ~left;
		case OpCode_Negate: return -left;
		}
		return Number{};
	}

	static bool TryFold(Instruction& instr, Number& outImm)
	{
		auto opCount = instr.OperandCount();
		if (opCount == 0) return false;

		auto immL = dyn_cast<Constant>(instr.GetOperand(0));
		if (!immL) return false;

		Constant* immR = nullptr;
		if (opCount > 1) immR = dyn_cast<Constant>(instr.GetOperand(1));

		auto code = instr.GetOpCode();
		switch (code)
		{
		case OpCode_Add: if (!immR) return false; outImm = immL->imm() + immR->imm(); return true;
		case OpCode_Subtract: if (!immR) return false; outImm = immL->imm() - immR->imm(); return true;
		case OpCode_Multiply: if (!immR) return false; outImm = immL->imm() * immR->imm(); return true;
		case OpCode_Divide: if (!immR) return false; outImm = immL->imm() / immR->imm(); return true;
		case OpCode_Modulus: if (!immR) return false; outImm = immL->imm() % immR->imm(); return true;
		case OpCode_BitwiseShiftLeft: if (!immR) return false; outImm = immL->imm() << immR->imm(); return true;
		case OpCode_BitwiseShiftRight: if (!immR) return false; outImm = immL->imm() >> immR->imm(); return true;
		case OpCode_AndAnd: if (!immR) return false; outImm = immL->imm().ToBool() && immR->imm().ToBool(); return true;
		case OpCode_OrOr: if (!immR) return false; outImm = immL->imm().ToBool() || immR->imm().ToBool(); return true;
		case OpCode_BitwiseAnd: if (!immR) return false; outImm = immL->imm() & immR->imm(); return true;
		case OpCode_BitwiseOr: if (!immR) return false; outImm = immL->imm() | immR->imm(); return true;
		case OpCode_BitwiseXor: if (!immR) return false; outImm = immL->imm() ^ immR->imm(); return true;
		case OpCode_LessThan: if (!immR) return false; outImm = immL->imm() < immR->imm(); return true;
		case OpCode_LessThanOrEqual: if (!immR) return false; outImm = immL->imm() <= immR->imm(); return true;
		case OpCode_GreaterThan: if (!immR) return false; outImm = immL->imm() > immR->imm(); return true;
		case OpCode_GreaterThanOrEqual: if (!immR) return false; outImm = immL->imm() >= immR->imm(); return true;
		case OpCode_Equal: if (!immR) return false; outImm = immL->imm() == immR->imm(); return true;
		case OpCode_NotEqual: if (!immR) return false; outImm = immL->imm() != immR->imm(); return true;
		case OpCode_LogicalNot: outImm = !immL->imm().ToBool(); return true;
		case OpCode_BitwiseNot: outImm = ~immL->imm(); return true;
		case OpCode_Negate: outImm = -immL->imm(); return true;
		}

		return false;
	}

#define DEFINE_IMM_COMP(name, value) \
	static bool name##(const Value* op) { \
	if (!Operand::IsImm(op)) return false; auto imm = cast<Constant>(op)->imm(); \
	switch (imm.Kind) { \
	case NumberType_Int8: return imm.i8 == value; \
	case NumberType_Int16: return imm.i16 == value; \
	case NumberType_Int32: return imm.i32 == value; \
	case NumberType_Int64: return imm.i64 == value; \
	case NumberType_UInt8: return imm.u8 == value; \
	case NumberType_UInt16: return imm.u16 == value; \
	case NumberType_UInt32: return imm.u32 == value; \
	case NumberType_UInt64: return imm.u64 == value; \
	case NumberType_Half: return imm.half_ == value; \
	case NumberType_Float: return imm.float_ == value; \
	case NumberType_Double: return imm.double_ == value; \
	default: break; } \
	return false; \
	}
}

#endif