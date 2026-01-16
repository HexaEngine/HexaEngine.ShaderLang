#include "il/instruction.hpp"
#include "il/il_context.hpp"
#include "il/il_text.hpp"

namespace HXSL
{
	namespace Backend
	{
		void Instruction::Dump() const
		{
			std::cout << ToString(*this, parent->GetParent()->GetMetadata()) << std::endl;
		}

		uint64_t Instruction::hash() const
		{
			XXHash3_64 hash{};
			hash.Combine(opcode);

			if (auto binary = dyn_cast<BinaryInstr>(this))
			{
				uint64_t leftHash = HXSL::Backend::hash(binary->GetLHS());
				uint64_t rightHash = HXSL::Backend::hash(binary->GetRHS());

				if (IsCommutative(opcode) && leftHash > rightHash)
				{
					std::swap(leftHash, rightHash);
				}

				hash.Combine(leftHash);
				hash.Combine(rightHash);
			}
			else
			{
				for (auto& operand : operands)
				{
					hash.Combine(HXSL::Backend::hash(operand));
				}
			}

			return hash.Finalize();
		}

		bool Instruction::operator==(const Instruction& other) const
		{
			if (opcode != other.opcode)
			{
				return false;
			}

			if (auto binary = dyn_cast<BinaryInstr>(this))
			{
				auto lhs = binary->GetLHS();
				auto rhs = binary->GetRHS();
				auto otherBinary = cast<BinaryInstr>(&other);
				auto lhsOther = otherBinary->GetLHS();
				auto rhsOther = otherBinary->GetRHS();
				if (!equals(lhs, lhsOther) || !equals(rhs, rhsOther))
				{
					if (!IsCommutative(opcode) || !equals(lhs, rhsOther) || !equals(rhs, lhsOther))
					{
						return false;
					}
				}
			}
			else
			{
				auto& otherOperands = other.GetOperands();
				const auto n = operands.size();
				if (n != otherOperands.size()) return false;
				for (size_t i = 0; i < n; ++i)
				{
					if (!equals(operands[i], otherOperands[i]))
					{
						return false;
					}
				}
			}

			return true;
		}

		static Operand* CloneOperand(BumpAllocator& alloc, const Value* operand)
		{
			if (operand == nullptr)
			{
				return nullptr;
			}

			switch (operand->GetTypeId())
			{
			case Value::ConstantVal:
			{
				auto constant = cast<Constant>(operand);
				return alloc.Alloc<Constant>(constant->imm());
			}
			case Value::VariableVal:
			{
				auto variable = cast<Variable>(operand);
				return alloc.Alloc<Variable>(variable->varId);
			}
			case Value::TypeVal:
			{
				auto typeValue = cast<TypeValue>(operand);
				return alloc.Alloc<TypeValue>(typeValue->typeId);
			}
			case Value::FuncVal:
			{
				auto function = cast<Function>(operand);
				return alloc.Alloc<Function>(function->funcId);
			}
			case Value::LabelVal:
			{
				auto label = cast<Label>(operand);
				return alloc.Alloc<Label>(label->label);
			}
			case Value::FieldVal:
			{
				auto fieldAccess = cast<FieldAccess>(operand);
				return alloc.Alloc<FieldAccess>(fieldAccess->field);
			}
			default:
				return nullptr;
			}
		}

		Instruction* Instruction::Clone(BumpAllocator& alloc) const
		{
			Instruction* cloned;

			switch (typeId)
			{
			case Value::BasicInstrVal:
			{
				cloned = alloc.Alloc<BasicInstr>(alloc, opcode);
				break;
			}
			case Value::ReturnInstrVal:
			{
				auto returnInstr = cast<ReturnInstr>(this);
				auto target = CloneOperand(alloc, returnInstr->GetOperand(0));
				if (target)
				{
					cloned = alloc.Alloc<ReturnInstr>(alloc, target);
				}
				else
				{
					cloned = alloc.Alloc<ReturnInstr>(alloc);
				}
				break;
			}
			case Value::CallInstrVal:
			{
				auto callInstr = cast<CallInstr>(this);
				auto function = CloneOperand(alloc, callInstr->GetOperand(0));
				if (callInstr->GetResult() != INVALID_VARIABLE)
				{
					cloned = alloc.Alloc<CallInstr>(alloc, callInstr->GetResult(), function);
				}
				else
				{
					cloned = alloc.Alloc<CallInstr>(alloc, function);
				}
				break;
			}
			case Value::JumpInstrVal:
			{
				auto jumpInstr = cast<JumpInstr>(this);
				auto label = cast<Label>(CloneOperand(alloc, jumpInstr->GetLabel()));
				cloned = alloc.Alloc<JumpInstr>(alloc, opcode, label);
				break;
			}
			case Value::BinaryInstrVal:
			{
				auto binaryInstr = cast<BinaryInstr>(this);
				auto lhs = CloneOperand(alloc, binaryInstr->GetLHS());
				auto rhs = CloneOperand(alloc, binaryInstr->GetRHS());
				cloned = alloc.Alloc<BinaryInstr>(alloc, opcode, binaryInstr->GetResult(), lhs, rhs);
				break;
			}
			case Value::UnaryInstrVal:
			{
				auto unaryInstr = cast<UnaryInstr>(this);
				auto operand = CloneOperand(alloc, unaryInstr->GetOperand());
				cloned = alloc.Alloc<UnaryInstr>(alloc, opcode, unaryInstr->GetResult(), operand);
				break;
			}
			case Value::StackAllocInstrVal:
			{
				auto stackAllocInstr = cast<StackAllocInstr>(this);
				auto typeValue = cast<TypeValue>(CloneOperand(alloc, stackAllocInstr->GetOperand(0)));
				cloned = alloc.Alloc<StackAllocInstr>(alloc, stackAllocInstr->GetResult(), typeValue);
				break;
			}
			case Value::OffsetInstrVal:
			{
				auto offsetInstr = cast<OffsetInstr>(this);
				auto src = cast<Variable>(CloneOperand(alloc, offsetInstr->GetOperand(0)));
				auto access = cast<FieldAccess>(CloneOperand(alloc, offsetInstr->GetOperand(1)));
				cloned = alloc.Alloc<OffsetInstr>(alloc, offsetInstr->GetResult(), src, access);
				break;
			}
			case Value::LoadInstrVal:
			{
				auto loadInstr = cast<LoadInstr>(this);
				auto src = CloneOperand(alloc, loadInstr->GetOperand(0));
				cloned = alloc.Alloc<LoadInstr>(alloc, loadInstr->GetResult(), src);
				break;
			}
			case Value::StoreInstrVal:
			{
				auto storeInstr = cast<StoreInstr>(this);
				auto dst = CloneOperand(alloc, storeInstr->GetOperand(0));
				auto src = CloneOperand(alloc, storeInstr->GetOperand(1));
				cloned = alloc.Alloc<StoreInstr>(alloc, dst, src);
				break;
			}
			case Value::LoadParamInstrVal:
			{
				auto loadParamInstr = cast<LoadParamInstr>(this);
				auto src = CloneOperand(alloc, loadParamInstr->GetOperand(0));
				cloned = alloc.Alloc<LoadParamInstr>(alloc, loadParamInstr->GetResult(), src);
				break;
			}
			case Value::StoreParamInstrVal:
			{
				auto storeParamInstr = cast<StoreParamInstr>(this);
				auto dst = CloneOperand(alloc, storeParamInstr->GetOperand(0));
				auto src = CloneOperand(alloc, storeParamInstr->GetOperand(1));
				cloned = alloc.Alloc<StoreParamInstr>(alloc, dst, src);
				break;
			}
			case Value::MoveInstrVal:
			{
				auto moveInstr = cast<MoveInstr>(this);
				auto src = CloneOperand(alloc, moveInstr->GetOperand(0));
				cloned = alloc.Alloc<MoveInstr>(alloc, moveInstr->GetResult(), src);
				break;
			}
			case Value::PhiInstrVal:
			{
				auto phiInstr = cast<PhiInstr>(this);
				auto phiClone = alloc.Alloc<PhiInstr>(alloc, phiInstr->GetResult(), phiInstr->OperandCount());
				for (size_t i = 0; i < phiInstr->OperandCount(); ++i)
				{
					phiClone->GetOperand(i) = CloneOperand(alloc, phiInstr->GetOperand(i));
				}
				cloned = phiClone;
				break;
			}
			default:
				HXSL_ASSERT(false, "Unknown instruction type in Clone()");
				return nullptr;
			}

			cloned->SetLocation(location);
			cloned->SetParent(parent);

			return cloned;
		}
	}
}