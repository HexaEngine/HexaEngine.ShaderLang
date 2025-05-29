#include "il_instruction.hpp"
#include "il_instruction.hpp"
#include "il_context.hpp"
#include "il_text.hpp"

namespace HXSL
{
	void Instruction::Dump() const
	{
		std::cout << ToString(*this, parent->GetParent()->GetMetadata()) << std::endl;
	}

	uint64_t Instruction::hash() const
	{
		return 0;
	}

	bool Instruction::operator==(const Instruction& other) const
	{
		return false;
	}
}