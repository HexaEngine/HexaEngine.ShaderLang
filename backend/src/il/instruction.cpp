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
	}
}