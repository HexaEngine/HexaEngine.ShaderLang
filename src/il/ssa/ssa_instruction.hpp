#ifndef SSA_INSTRUCTION_HPP
#define SSA_INSTRUCTION_HPP

#include "il/instruction.hpp"

namespace HXSL
{
	namespace Backend
	{
		constexpr uint64_t SSA_VARIABLE_MASK = 0xFFFFFFFF;

		constexpr uint32_t SSA_VERSION_MASK = 0x7FFFFFFF;
		constexpr uint8_t SSA_VERSION_SHIFT = 32;

		constexpr uint64_t SSA_TEMP_FLAG_MASK = 0x7FFFFFFFFFFFFFFF;
		constexpr uint8_t SSA_TEMP_FLAG_SHIFT = 63;
		constexpr uint64_t SSA_VARIABLE_TEMP_FLAG = static_cast<uint64_t>(1) << SSA_TEMP_FLAG_SHIFT;

		constexpr uint64_t SSA_VERSION_STRIP_MASK = SSA_VARIABLE_TEMP_FLAG | SSA_VARIABLE_MASK;

		inline static uint64_t MakeVersion(uint64_t varId, uint32_t version)
		{
			uint64_t result = varId & SSA_VARIABLE_MASK;
			result |= (static_cast<uint64_t>(version & SSA_VERSION_MASK) << SSA_VERSION_SHIFT);
			result |= varId & SSA_VARIABLE_TEMP_FLAG;
			return result;
		}

		inline static uint32_t ExtractVarId(uint64_t varIdVer)
		{
			return static_cast<uint32_t>(varIdVer & SSA_VARIABLE_MASK);
		}

		inline static uint32_t ExtractVersion(uint64_t varIdVer)
		{
			return static_cast<uint32_t>(varIdVer >> SSA_VERSION_SHIFT) & SSA_VERSION_MASK;
		}

		inline static bool ExtractTempFlag(uint64_t varIdVer)
		{
			return varIdVer & SSA_VARIABLE_TEMP_FLAG;
		}

		inline static bool IsTempVar(uint64_t varIdVer) { return ExtractTempFlag(varIdVer); }

		inline static void DecomposeVariableID(uint64_t varIdVer, uint32_t& varId, uint32_t& version, bool& isTemp)
		{
			varId = ExtractVarId(varIdVer);
			version = ExtractVersion(varIdVer);
			isTemp = ExtractTempFlag(varIdVer);
		}
	}
}

#endif