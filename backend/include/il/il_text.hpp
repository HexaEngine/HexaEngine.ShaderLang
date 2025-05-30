#ifndef IL_TEXT_HPP
#define IL_TEXT_HPP

#include "instruction.hpp"
#include "ssa/ssa_instruction.hpp"
#include "il_metadata.hpp"

namespace HXSL
{
	namespace Backend
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

			case OpCode_StoreParam: return "starg";
			case OpCode_LoadParam: return "ldarg";
			case OpCode_StoreRefParam: return "strefarg";
			case OpCode_LoadRefParam: return "ldrefarg";

			case OpCode_Call: return "call";

			case OpCode_Discard: return "discard";

			case OpCode_Phi: return "phi";

			case OpCode_Add: return "add";
			case OpCode_Subtract: return "sub";
			case OpCode_Multiply: return "mul";
			case OpCode_Divide: return "div";
			case OpCode_Modulus: return "rem";
			case OpCode_BitwiseShiftLeft: return "bls";
			case OpCode_BitwiseShiftRight: return "brs";
			case OpCode_AndAnd: return "land";
			case OpCode_OrOr: return "lor";
			case OpCode_BitwiseAnd: return "and";
			case OpCode_BitwiseOr: return "or";
			case OpCode_BitwiseXor: return "xor";
			case OpCode_LessThan: return "lt";
			case OpCode_LessThanOrEqual: return "ltq";
			case OpCode_GreaterThan: return "gt";
			case OpCode_GreaterThanOrEqual: return "gtq";
			case OpCode_Equal: return "eq";
			case OpCode_NotEqual: return "neq";
			case OpCode_Increment: return "inc";
			case OpCode_Decrement: return "dec";
			case OpCode_LogicalNot: return "lnot";
			case OpCode_BitwiseNot: return "not";
			case OpCode_Negate: return "neg";

			case OpCode_VecExtract: return "v_extr";
			case OpCode_VecSetX: return "v_setx";
			case OpCode_VecSetY: return "v_sety";
			case OpCode_VecSetZ: return "v_setz";
			case OpCode_VecSetW: return "v_setw";

			case OpCode_BroadcastVec: return "vec_bcast";

			case OpCode_VecSwizzle: return "vec_swiz";

			case OpCode_VecAdd: return "vec_add";
			case OpCode_VecSubtract: return "vec_sub";
			case OpCode_VecMultiply: return "vec_mul";
			case OpCode_VecDivide: return "vec_div";
			case OpCode_VecFusedMultiplyAdd: return "vec_fma";

			default: return "Unknown OpCode";
			}
		}

		static std::string ToString(ILVarId varId, const ILMetadata& metadata)
		{
			std::ostringstream oss;
			oss << (varId.var.temp ? "%tmp" : "%var") << varId.var.version << "_" << varId.var.id << ": " << metadata.GetVarTypeName(varId);
			return oss.str();
		}

		static std::string ToString(const Value* operand, bool first, const ILMetadata& metadata)
		{
			std::ostringstream oss;
			if (!first)
			{
				oss << ", ";
			}

			switch (operand->GetTypeId())
			{
			case Value::ConstantVal:
				oss << cast<Constant>(operand)->imm().ToString();
				break;
			case Value::VariableVal:
			{
				auto op = cast<Variable>(operand);
				oss << ToString(op->varId, metadata);
			}
			break;
			case Value::FieldVal:
			{
				auto op = cast<FieldAccess>(operand);
				oss << metadata.GetTypeName(op->field.typeId) << "::" << metadata.GetFieldName(op->field);
			}
			break;
			case Value::LabelVal:
			{
				auto op = cast<Label>(operand);
				oss << "#loc_" << op->label.value;
			}
			break;
			case Value::FuncVal:
			{
				auto op = cast<Function>(operand);
				oss << metadata.GetFuncName(op->funcId);
			}
			break;
			case Value::TypeVal:
			{
				auto op = cast<TypeValue>(operand);
				oss << metadata.GetTypeName(op->typeId);
			}
			break;
			}

			return oss.str();
		}

		static std::string ToString(const Instruction& instruction, const ILMetadata& metadata)
		{
			auto instr = &instruction;
			std::ostringstream oss;

			if (auto res = dyn_cast<ResultInstr>(instr))
			{
				oss << ToString(res->GetResult(), metadata);
				oss << " = ";
			}

			oss << OpCodeToString(instr->GetOpCode()) + " ";

			bool first = true;
			auto n = instr->OperandCount();
			for (size_t i = 0; i < n; ++i)
			{
				auto op = instr->GetOperand(i);
				oss << ToString(op, first, metadata);
				first = false;
			}

			if (auto loc = instr->GetLocation())
			{
				auto& span = *loc;
				auto len = oss.view().length();
				for (size_t i = len; i < 75; ++i) oss << " ";
				oss << "; " << " (Line: " << span.line << " Column: " << span.column << ")";
			}

			return oss.str();
		}
	}
}

#endif