#ifndef JUMP_TABLE_HPP
#define JUMP_TABLE_HPP

#include "pch/ast.hpp"
#include "il_instruction.hpp"

namespace HXSL
{
	constexpr ILInstruction* INVALID_JUMP_LOCATION_PTR = nullptr;
	constexpr ILLabel INVALID_JUMP_LOCATION = ILLabel(static_cast<uint64_t>(-1));

	struct JumpTable
	{
		std::vector<ILInstruction*> locations;

		ILLabel Allocate(ILInstruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			auto idx = locations.size();
			locations.push_back(location);
			return ILLabel(idx);
		}

		void SetLocation(ILLabel id, ILInstruction* location)
		{
			locations[id.value] = location;
		}

		void Prepare()
		{
			for (auto& p : locations)
			{
				p = p->GetNext();
			}
		}

		ILInstruction* GetLocation(ILLabel id) const
		{
			return locations[id.value];
		}
	};
}

#endif