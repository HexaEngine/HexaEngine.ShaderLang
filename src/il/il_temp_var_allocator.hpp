#ifndef IL_TEMP_VAR_ALLOCATOR_HPP
#define IL_TEMP_VAR_ALLOCATOR_HPP

#include "il_instruction.hpp"
#include "il_container.hpp"

namespace HXSL
{
	struct ILTempVariableAllocator
	{
		ILRegister current = 0;

		ILRegister Alloc(SymbolDef* type)
		{
			return current.id++;
		}

		void Free(ILRegister reg)
		{
			if (reg == INVALID_REGISTER) return;
			if (current.id - 1 != reg.id)
			{
				HXSL_ASSERT(false, "");
			}

			current = current.id - 1;
		}
	};
}

#endif