#ifndef IL_TEXT_HPP
#define IL_TEXT_HPP

#include "il_instruction.hpp"
#include "ssa/ssa_instruction.hpp"
#include "il_metadata.hpp"

namespace HXSL
{
	static std::string OpCodeToString(ILOpCode opcode)
	{
		switch (opcode)
		{
		case OpCode_Noop: return "nop";
		case OpCode_StackAlloc: return "alloca";
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
		case OpCode_StoreParamRef: return "rfarg";
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

	static std::string OperandKindToString(ILOperandKind kind)
	{
		switch (kind.value)
		{
		case ILOperandKind_Register: return "Register";
		case ILOperandKind_Variable: return "Variable";
		case ILOperandKind_Field: return "Field";
		case ILOperandKind_Label: return "Label";
		case ILOperandKind_Type: return "Type";
		case ILOperandKind_Func: return "Func";
		case ILOperandKind_Imm_i8: return "i8";
		case ILOperandKind_Imm_u8: return "u8";
		case ILOperandKind_Imm_i16: return "i16";
		case ILOperandKind_Imm_u16: return "u16";
		case ILOperandKind_Imm_i32: return "i32";
		case ILOperandKind_Imm_u32: return "u32";
		case ILOperandKind_Imm_i64: return "i64";
		case ILOperandKind_Imm_u64: return "u64";
		case ILOperandKind_Imm_f16: return "f16";
		case ILOperandKind_Imm_f32: return "f32";
		case ILOperandKind_Imm_f64: return "f64";
		default: return "Unknown";
		}
	}

	static std::string ToString(const ILOperand& operand, bool first, const ILMetadata& metadata)
	{
		std::ostringstream oss;
		if (!first)
		{
			oss << ", ";
		}

		switch (operand.kind.value)
		{
		case ILOperandKind_Register:
			oss << "%tmp" << operand.reg.id;
			break;
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
			oss << operand.imm().ToString();
			break;
		case ILOperandKind_Variable:
		{
			oss << "%var" << (operand.varId & SSA_VARIABLE_MASK) << "_" << (operand.varId >> 32) << ": " << metadata.GetVarTypeName(operand.varId);
		}
		break;
		case ILOperandKind_Field:
			oss << metadata.GetTypeName(operand.field.typeId) << "::" << operand.field.fieldId;
			break;
		case ILOperandKind_Label:
			oss << "#loc_" << operand.varId;
			break;
		case ILOperandKind_Func:
			oss << metadata.GetFuncName(operand.varId);
			break;
		case ILOperandKind_Type:
			oss << metadata.GetTypeName(operand.varId);
			break;
		case ILOperandKind_Phi:
		{
			auto& phi = metadata.phiMetadata[operand.varId];
			for (auto& p : phi.params)
			{
				oss << "[";
				oss << "%var" << (p & SSA_VARIABLE_MASK) << "_" << (p >> 32);
				oss << "]";
			}
		}
		break;
		}

		return oss.str();
	}

	static std::string GetOpKindType(ILOpKind opKind)
	{
		opKind &= static_cast<ILOpKind>(ILOpKindTypeMask);
		return OpKindToString(opKind);
	}

	static std::string ToString(const ILInstruction& instruction, const ILMetadata& metadata)
	{
		std::ostringstream oss;

		if (instruction.operandResult.kind != ILOperandKind_Disabled)
		{
			oss << ToString(instruction.operandResult, true, metadata);
			oss << " = ";
		}

		oss << OpCodeToString(instruction.opcode) + " ";

		bool first = true;
		if (instruction.operandLeft.kind != ILOperandKind_Disabled)
		{
			oss << ToString(instruction.operandLeft, first, metadata);
			first = false;
		}

		if (instruction.operandRight.kind != ILOperandKind_Disabled)
		{
			oss << ToString(instruction.operandRight, first, metadata);
			first = false;
		}

		if (IsFlagSet(instruction.opKind, ILOpKind_Const))
		{
			oss << " const";
		}

		if (IsFlagSet(instruction.opKind, ILOpKind_Precise))
		{
			oss << " precise";
		}

		oss << " " << GetOpKindType(instruction.opKind);

		return oss.str();
	}
}

#endif