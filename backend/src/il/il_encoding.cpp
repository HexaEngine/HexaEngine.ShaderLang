#include "il/il_encoding.hpp"
#include "utils/endianness.hpp"

namespace HXSL
{
    namespace Backend
    {
        void ILWriter::EncodeOpCode(ILOpCode code)
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

        void ILWriter::EncodeVarId(ILVarId varId)
        {
            Write(EndianUtils::ToLittleEndian(varId.raw));
        }

        void ILWriter::EncodeOperand(const Operand* op)
        {
            ValueId kind = op->GetTypeId();
            switch (kind)
            {
            case Value::LabelVal:
            {

                auto label = cast<Label>(op);
                Write(EndianUtils::ToLittleEndian(label->label.value));
            }
            break;
            case Value::TypeVal:
            {
                auto typeValue = cast<TypeValue>(op);
                Write(EndianUtils::ToLittleEndian(typeValue->typeId->id));
            }
            break;
            case Value::FuncVal:
            {
                auto function = cast<Function>(op);
                Write(EndianUtils::ToLittleEndian(function->funcId->id));
            }
            break;
            case Value::FieldVal:
            {
                auto field = cast<FieldAccess>(op);
                Write(EndianUtils::ToLittleEndian(field->field.fieldId.value));
                Write(EndianUtils::ToLittleEndian(field->field.typeId->id));
            }
            break;
            case Value::VariableVal:
            {
                auto variable = cast<Variable>(op);
                Write(EndianUtils::ToLittleEndian(variable->varId.raw));
            }
            break;
            case Value::ConstantVal:
            {
                auto constant = cast<Constant>(op);
                EncodeImmediate(constant->imm());
            }
            break;
            default:
                HXSL_ASSERT(false, "Unsupported operand type for encoding.");
                break;
            }
        }

        void ILWriter::EncodeImmediate(const Number& imm)
        {
            switch (imm.Kind)
            {
            case NumberType_Int8:
            case NumberType_UInt8:
                WriteByte(imm.u8);
                break;
            case NumberType_Int16:
            case NumberType_UInt16:
            case NumberType_Half:
                Write(EndianUtils::ToLittleEndian(imm.u16));
                break;
            case NumberType_Int32:
            case NumberType_UInt32:
            case NumberType_Float:
                Write(EndianUtils::ToLittleEndian(imm.u32));
                break;
            case NumberType_Int64:
            case NumberType_UInt64:
            case NumberType_Double:
                Write(EndianUtils::ToLittleEndian(imm.u64));
                break;
            default:
                break;
            }
        }

        static OpKind NumberTypeToOpKind(NumberType type)
        {
            switch (type)
            {
            case NumberType_UInt8: return OpKind::ImmU8;
            case NumberType_UInt16: return OpKind::ImmU16;
            case NumberType_UInt32: return OpKind::ImmU32;
            case NumberType_UInt64: return OpKind::ImmU64;
            case NumberType_Int8: return OpKind::ImmI8;
            case NumberType_Int16: return OpKind::ImmI16;
            case NumberType_Int32: return OpKind::ImmI32;
            case NumberType_Int64: return OpKind::ImmI64;
            case NumberType_Half: return OpKind::ImmF16;
            case NumberType_Float: return OpKind::ImmF32;
            case NumberType_Double: return OpKind::ImmF64;
            default:
                HXSL_ASSERT(false, "Unsupported immediate type for encoding.");
                return OpKind::Disabled;
            }
        }

        static OpKind GetOperandKind(const Operand* op)
        {
            if (op == nullptr) return OpKind::Disabled;
            auto kind = op->GetTypeId();
            switch (kind)
            {
            case Value::VariableVal: return OpKind::Variable;
            case Value::ConstantVal: return NumberTypeToOpKind(cast<Constant>(op)->imm().Kind);
            case Value::LabelVal: return OpKind::Label;
            case Value::TypeVal: return OpKind::Type;
            case Value::FuncVal: return OpKind::Function;
            case Value::FieldVal: return OpKind::Field;
            default: HXSL_ASSERT(false, "Unsupported operand type for encoding."); return OpKind::Disabled;
            }
        }

        void ILWriter::Write(const Instruction& instr)
        {
            EncodeOpCode(instr.GetOpCode());
            auto pos = stream->Position();
            auto kind = instr.GetTypeId();
            switch (kind)
            {
            case Value::BasicInstrVal:
            {
                // Nothing to do.
            }
            break;
            case Value::ReturnInstrVal:
            {
                auto returnInstr = cast<ReturnInstr>(&instr);
                auto val = returnInstr->GetReturnValue();
                Write(GetOperandKind(val));
                if (val)
                {
                    EncodeOperand(val);
                }
            }
            break;
            case Value::CallInstrVal:
            {
                auto callInstr = cast<CallInstr>(&instr);
                auto func = callInstr->GetFunction();
                EncodeOperand(func);
                Write(EndianUtils::ToLittleEndian(callInstr->GetResult().raw));
            }
            break;
            case Value::JumpInstrVal:
            {
                auto jumpInstr = cast<JumpInstr>(&instr);
                auto label = jumpInstr->GetLabel();
                EncodeOperand(label);
            }
            break;
            case Value::BinaryInstrVal:
            {
                auto binaryInstr = cast<BinaryInstr>(&instr);
                auto lhs = binaryInstr->GetLHS();
                auto rhs = binaryInstr->GetRHS();
                auto dst = binaryInstr->GetResult();
                auto lhsKind = GetOperandKind(lhs);
                auto rhsKind = GetOperandKind(rhs);
                auto combinedKind = static_cast<uint8_t>(static_cast<uint8_t>(lhsKind) | (static_cast<uint8_t>(rhsKind) << OpKindBits));
                Write(combinedKind);
                EncodeOperand(lhs);
                EncodeOperand(rhs);
                EncodeVarId(dst);
            }
            break;
            case Value::UnaryInstrVal:
            {
                auto unaryInstr = cast<UnaryInstr>(&instr);
                auto op = unaryInstr->GetOperand();
                auto dst = unaryInstr->GetResult();
                Write(GetOperandKind(op));
                EncodeOperand(op);
                EncodeVarId(dst);
            }
            break;
            case Value::StackAllocInstrVal:
            {
                auto stackAllocInstr = cast<StackAllocInstr>(&instr);
                auto typeId = stackAllocInstr->GetOperand(0);
                EncodeOperand(typeId);
                EncodeVarId(stackAllocInstr->GetResult());
            }
            break;
            case Value::OffsetInstrVal:
            {
                auto offsetInstr = cast<OffsetInstr>(&instr);
                auto src = offsetInstr->GetOperand(0);
                auto field = offsetInstr->GetOperand(1);
                EncodeOperand(src);
                EncodeOperand(field);
                EncodeVarId(offsetInstr->GetResult());
            }
            break;
            case Value::LoadInstrVal:
            {
                auto loadInstr = cast<LoadInstr>(&instr);
                auto src = loadInstr->GetOperand(0);
                EncodeOperand(src);
                EncodeVarId(loadInstr->GetResult());
            }
            break;
            case Value::StoreInstrVal:
            {
                auto storeInstr = cast<StoreInstr>(&instr);
                auto dst = storeInstr->GetDestination();
                auto src = storeInstr->GetSource();
                auto srcKind = GetOperandKind(src);
                Write(srcKind);
                EncodeOperand(dst);
                EncodeOperand(src);
            }
            break;
            case Value::LoadParamInstrVal:
            {
                auto loadParamInstr = cast<LoadParamInstr>(&instr);
                auto src = loadParamInstr->GetSource();
                Write(GetOperandKind(src));
                EncodeOperand(src);
                EncodeVarId(loadParamInstr->GetResult());
            }
            break;
            case Value::StoreParamInstrVal:
            {
                auto storeParamInstr = cast<StoreParamInstr>(&instr);
                auto src = storeParamInstr->GetSource();
                auto dst = storeParamInstr->GetDestination();
                auto srcKind = GetOperandKind(src);
                auto dstKind = GetOperandKind(dst);
                auto combinedKind = static_cast<uint8_t>(srcKind) | (static_cast<uint8_t>(dstKind) << OpKindBits);
                Write(combinedKind);
                EncodeOperand(src);
                EncodeOperand(dst);
            }
            break;
            case Value::MoveInstrVal:
            {
                auto moveInstr = cast<MoveInstr>(&instr);
                auto src = moveInstr->GetSource();
                auto dst = moveInstr->GetResult();
                auto srcKind = GetOperandKind(src);
                Write(srcKind);
                EncodeOperand(src);
                EncodeVarId(dst);
            }
            break;
            default:
                HXSL_ASSERT(false, "Unsupported instruction type for encoding.");
                break;
            }
        }

        ILOpCode ILReader::DecodeOpCode()
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

        ILVarId ILReader::DecodeVarId()
        {
            auto raw = EndianUtils::FromLittleEndian(Read<uint64_t>());
            return ILVarId(raw);
        }

        Operand* ILReader::DecodeOperand(OpKind kind)
        {
            auto& alloc = options.allocator;
            switch (kind)
            {
            case OpKind::Variable:
            {
                auto varId = DecodeVarId();
                return alloc.Alloc<Variable>(varId);
            }
            case OpKind::ImmU8:
                return alloc.Alloc<Constant>(Number(ReadByte()));
            case OpKind::ImmI8:
                return alloc.Alloc<Constant>(Number(Read<int8_t>()));
            case OpKind::ImmU16:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<uint16_t>())));
            case OpKind::ImmI16:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<int16_t>())));
            case OpKind::ImmF16:
                return alloc.Alloc<Constant>(Number(static_cast<half>(EndianUtils::FromLittleEndian(Read<uint16_t>()))));
            case OpKind::ImmU32:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<uint32_t>())));
            case OpKind::ImmI32:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<int32_t>())));
            case OpKind::ImmF32:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<float>())));
            case OpKind::ImmU64:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<uint64_t>())));
            case OpKind::ImmI64:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<int64_t>())));
            case OpKind::ImmF64:
                return alloc.Alloc<Constant>(Number(EndianUtils::FromLittleEndian(Read<double>())));
            case OpKind::Label:
            {
                auto labelValue = EndianUtils::FromLittleEndian(Read<uint64_t>());
                return alloc.Alloc<Label>(ILLabel(labelValue));

            }
            case OpKind::Type:
            {
                auto typeId = EndianUtils::FromLittleEndian(Read<ILTypeMetadata::ILTypeId>());
                auto type = options.metadata.GetTypeById(typeId);
                return alloc.Alloc<TypeValue>(type);
            }
            case OpKind::Function:
            {
                auto funcId = EndianUtils::FromLittleEndian(Read<ILFuncCallMetadata::ILFuncCallId>());
                auto func = options.metadata.GetFuncById(funcId);
                return alloc.Alloc<Function>(func);
            }
            case OpKind::Field:
            {
                auto fieldIdValue = EndianUtils::FromLittleEndian(Read<uint32_t>());
                auto typeId = EndianUtils::FromLittleEndian(Read<ILTypeMetadata::ILTypeId>());
                auto type = options.metadata.GetTypeById(typeId);
                ILFieldAccess field(type, ILFieldId(fieldIdValue));
                return alloc.Alloc<FieldAccess>(field);
            }
            default:
                HXSL_ASSERT(false, "Unsupported operand kind for decoding.");
                return nullptr;
            }
        }

        Instruction* ILReader::Read()
        {
			auto pos = stream->Position();
            auto& alloc = options.allocator;
            ILOpCode opcode = DecodeOpCode();
            if (IsBasic(opcode))
            {
                return alloc.Alloc<BasicInstr>(alloc, opcode);
            }
            else if (IsReturn(opcode))
            {
                auto kind = static_cast<OpKind>(ReadByte());
                Operand* val = nullptr;
                if (kind != OpKind::Disabled)
                {
                    val = DecodeOperand(kind);
                }
                return alloc.Alloc<ReturnInstr>(alloc, val);
            }
            else if (IsCall(opcode))
            {
                auto func = DecodeOperand(OpKind::Function);
                auto dst = DecodeVarId();
                return alloc.Alloc<CallInstr>(alloc, dst, func);
            }
            else if (IsJump(opcode))
            {
                auto label = cast<Label>(DecodeOperand(OpKind::Label));
                return alloc.Alloc<JumpInstr>(alloc, opcode, label);
            }
            else if (IsBinary(opcode))
            {
                uint8_t combinedKind = ReadByte();
                auto lhsKind = static_cast<OpKind>(combinedKind & OpKindMask);
                auto rhsKind = static_cast<OpKind>((combinedKind >> OpKindBits) & OpKindMask);
                auto lhs = DecodeOperand(lhsKind);
                auto rhs = DecodeOperand(rhsKind);
                auto dst = DecodeVarId();
                return alloc.Alloc<BinaryInstr>(alloc, opcode, dst, lhs, rhs);
            }
            else if (IsUnary(opcode))
            {
                auto kind = static_cast<OpKind>(ReadByte());
                auto op = DecodeOperand(kind);
                auto dst = DecodeVarId();
                return alloc.Alloc<UnaryInstr>(alloc, opcode, dst, op);
            }
            else if (opcode == OpCode_StackAlloc)
            {
                auto typeId = cast<TypeValue>(DecodeOperand(OpKind::Type));
                auto dst = DecodeVarId();
                return alloc.Alloc<StackAllocInstr>(alloc, dst, typeId);
            }
            else if (opcode == OpCode_OffsetAddress)
            {
                auto src = cast<Variable>(DecodeOperand(OpKind::Variable));
                auto field = cast<FieldAccess>(DecodeOperand(OpKind::Field));
                auto dst = DecodeVarId();
                return alloc.Alloc<OffsetInstr>(alloc, dst, src, field);
            }
            else if (opcode == OpCode_Load)
            {
                auto src = cast<Variable>(DecodeOperand(OpKind::Variable));
                auto dst = DecodeVarId();
                return alloc.Alloc<LoadInstr>(alloc, dst, src);
            }
            else if (opcode == OpCode_Store)
            {
                auto srcKind = static_cast<OpKind>(ReadByte());
                auto dst = cast<Variable>(DecodeOperand(OpKind::Variable));
                auto src = DecodeOperand(srcKind);
                return alloc.Alloc<StoreInstr>(alloc, dst, src);
            }
            else if (opcode == OpCode_LoadParam)
            {
                auto srcKind = static_cast<OpKind>(ReadByte());
                auto src = DecodeOperand(srcKind);
                auto dst = DecodeVarId();
                return alloc.Alloc<LoadParamInstr>(alloc, dst, src);
            }
            else if (opcode == OpCode_StoreParam)
            {
                uint8_t combinedKind = ReadByte();
                auto srcKind = static_cast<OpKind>(combinedKind & OpKindMask);
                auto dstKind = static_cast<OpKind>((combinedKind >> OpKindBits) & OpKindMask);
                auto src = DecodeOperand(srcKind);
                auto dst = DecodeOperand(dstKind);
                return alloc.Alloc<StoreParamInstr>(alloc, dst, src);
            }
            else if (opcode == OpCode_Move)
            {
                auto srcKind = static_cast<OpKind>(ReadByte());
                auto src = DecodeOperand(srcKind);
                auto dst = DecodeVarId();
                return alloc.Alloc<MoveInstr>(alloc, dst, src);
            }

            HXSL_ASSERT(false, "Unsupported opcode for decoding.");
            return nullptr;
        }
    }
}