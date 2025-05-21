#ifndef SSA_INSTRUCTION_HPP
#define SSA_INSTRUCTION_HPP

#include "il/il_instruction.hpp"

namespace HXSL
{
	constexpr uint64_t SSA_VARIABLE_MASK = 0xFFFFFFFF;

	static uint64_t MakeVersion(uint64_t varId, uint32_t version)
	{
		return (varId & SSA_VARIABLE_MASK) | (static_cast<uint64_t>(version) << 32);
	}
}

#endif