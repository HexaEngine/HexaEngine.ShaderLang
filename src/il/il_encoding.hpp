#ifndef IL_ENCODING_HPP
#define IL_ENCODING_HPP

#include "pch/il.hpp"
#include "utils/endianness.hpp"
#include "io/stream.hpp"

namespace HXSL
{
	class ILEncoder
	{
		Stream* stream;

		void Write(const void* src, size_t s)
		{
			auto result = stream->Write(src, s);
			if (result == EOF)
			{
				throw std::runtime_error("Unexpected error encountered while writing to a stream.");
			}
		}

		void WriteByte(uint8_t value)
		{
			Write(&value, 1);
		}

		template <typename T>
		void Write(const T& value)
		{
			Write(&value, sizeof(T));
		}

		void EncodeOpCode(ILOpCode code)
		{
			uint64_t value = static_cast<uint64_t>(code);
			do
			{
				uint8_t b = value & 0x7F;
				value >>= 7;
				b |= (value != 0) << 7;
				WriteByte(b);
			} while (value);
		}

		void EncodeOperand(const ILOperand& op)
		{
			ILOperandKind_T kind = op.kind;
			switch (kind)
			{
			case ILOperandKind_Register:
			case ILOperandKind_Variable:
			case ILOperandKind_Label:
			case ILOperandKind_Type:
			case ILOperandKind_Func:
			{
				Write(EndianUtils::ToLittleEndian(static_cast<uint32_t>(op.varId & SSA_VARIABLE_MASK)));
			}
			break;
			case ILOperandKind_Field:
			{
				auto v0 = EndianUtils::ToLittleEndian(op.field.typeId);
				auto v1 = EndianUtils::ToLittleEndian(op.field.fieldId);
				uint64_t v = static_cast<uint64_t>(v0) | static_cast<uint64_t>(v1 << 32);
				Write(v);
			}
			break;
			case ILOperandKind_Imm_i8:
			case ILOperandKind_Imm_u8:
			{
				WriteByte(op.imm_m.u8);
			}
			break;
			case ILOperandKind_Imm_i16:
			case ILOperandKind_Imm_u16:
			case ILOperandKind_Imm_f16:
			{
				Write(EndianUtils::ToLittleEndian(op.imm_m.u16));
			}
			break;
			case ILOperandKind_Imm_i32:
			case ILOperandKind_Imm_u32:
			case ILOperandKind_Imm_f32:
			{
				Write(EndianUtils::ToLittleEndian(op.imm_m.u32));
			}
			break;
			case ILOperandKind_Imm_i64:
			case ILOperandKind_Imm_u64:
			case ILOperandKind_Imm_f64:
			{
				Write(EndianUtils::ToLittleEndian(op.imm_m.u64));
			}
			break;
			}
		}

	public:
		ILEncoder(Stream* stream) : stream(stream) {}

		void Encode(const ILInstruction& instr)
		{
			EncodeOpCode(instr.opcode);
			WriteByte(instr.opKind);
			uint16_t operandKind = 0;
			operandKind |= (instr.operandLeft.kind.value & 0x1F);
			operandKind |= (instr.operandRight.kind.value & 0x1F) << 5;
			operandKind |= (instr.operandResult.kind.value & 0x1F) << 10;
			Write(EndianUtils::ToLittleEndian(operandKind));
			EncodeOperand(instr.operandLeft);
			EncodeOperand(instr.operandRight);
			EncodeOperand(instr.operandResult);
		}
	};

	class ILDecoder
	{
		Stream* stream;

		void Read(void* dst, size_t s)
		{
			auto read = stream->Read(dst, s);
			if (read != s)
			{
				throw std::runtime_error("Unexpected end of stream.");
			}
		}

		uint8_t ReadByte()
		{
			uint8_t val;
			Read(&val, 1);
			return val;
		}

		template <typename T>
		T Read()
		{
			T dst{};
			Read(&dst, sizeof(T));
			return dst;
		}

		ILOpCode DecodeOpCode()
		{
			uint64_t value = 0;
			size_t shift = 0;
			uint8_t b;
			do
			{
				b = ReadByte();
				value |= static_cast<uint64_t>(b & 0x7F) << shift;
				shift += 7;
			} while (b & 0x80);

			return static_cast<ILOpCode>(value);
		}

		void DecodeOperand(ILOperand& op)
		{
			ILOperandKind_T kind = op.kind;
			switch (kind)
			{
			case ILOperandKind_Register:
			case ILOperandKind_Variable:
			case ILOperandKind_Label:
			case ILOperandKind_Type:
			case ILOperandKind_Func:
			{
				op.varId = static_cast<uint64_t>(EndianUtils::FromLittleEndian(Read<uint32_t>()));
			}
			break;
			case ILOperandKind_Field:
			{
				uint64_t v = Read<uint64_t>();
				op.field.typeId = EndianUtils::FromLittleEndian(static_cast<uint32_t>(v & 0xFFFFFFFF));
				op.field.fieldId = EndianUtils::FromLittleEndian(static_cast<uint32_t>((v >> 32) & 0xFFFFFFFF));
			}
			break;
			case ILOperandKind_Imm_i8:
			case ILOperandKind_Imm_u8:
			{
				op.imm_m.u8 = ReadByte();
			}
			break;
			case ILOperandKind_Imm_i16:
			case ILOperandKind_Imm_u16:
			case ILOperandKind_Imm_f16:
			{
				op.imm_m.u16 = EndianUtils::FromLittleEndian(Read<uint16_t>());
			}
			break;
			case ILOperandKind_Imm_i32:
			case ILOperandKind_Imm_u32:
			case ILOperandKind_Imm_f32:
			{
				op.imm_m.u32 = EndianUtils::FromLittleEndian(Read<uint32_t>());
			}
			break;
			case ILOperandKind_Imm_i64:
			case ILOperandKind_Imm_u64:
			case ILOperandKind_Imm_f64:
			{
				op.imm_m.u64 = EndianUtils::FromLittleEndian(Read<uint64_t>());
			}
			break;
			}
		}

	public:
		ILDecoder(Stream* stream) : stream(stream) {}

		bool Decode(ILInstruction& instr)
		{
			instr.opcode = DecodeOpCode();
			instr.opKind = ReadByte();
			uint16_t v = EndianUtils::FromLittleEndian(Read<uint16_t>());
			instr.operandLeft.kind = static_cast<ILOperandKind_T>(v & 0x1F);
			instr.operandRight.kind = static_cast<ILOperandKind_T>((v >> 5) & 0x1F);
			instr.operandResult.kind = static_cast<ILOperandKind_T>((v >> 10) & 0x1F);
			DecodeOperand(instr.operandLeft);
			DecodeOperand(instr.operandRight);
			DecodeOperand(instr.operandResult);
		}
	};
}

#endif