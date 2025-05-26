#ifndef JUMP_TABLE_HPP
#define JUMP_TABLE_HPP

#include "pch/ast.hpp"
#include "il_instruction.hpp"

namespace HXSL
{
	constexpr ILInstruction* INVALID_JUMP_LOCATION_PTR = nullptr;
	constexpr uint64_t INVALID_JUMP_LOCATION = -1;

	struct JumpTable
	{
		std::vector<ILInstruction*> locations;

		uint64_t Allocate(ILInstruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			auto idx = locations.size();
			locations.push_back(location);
			return idx;
		}

		void SetLocation(uint64_t id, ILInstruction* location)
		{
			locations[id] = location;
		}

		void Prepare()
		{
			for (auto& p : locations)
			{
				p = p->next;
			}
		}

		ILInstruction* GetLocation(uint64_t id) const
		{
			return locations[id];
		}
	};
}

#endif