#ifndef JUMP_TABLE_HPP
#define JUMP_TABLE_HPP

#include "pch/ast.hpp"
#include "instruction.hpp"

namespace HXSL
{
	namespace Backend
	{
		constexpr Instruction* INVALID_JUMP_LOCATION_PTR = nullptr;
		constexpr ILLabel INVALID_JUMP_LOCATION = ILLabel(static_cast<uint64_t>(-1));

		struct JumpTable
		{
			std::vector<Instruction*> locations;

			ILLabel Allocate(Instruction* location = INVALID_JUMP_LOCATION_PTR)
			{
				auto idx = locations.size();
				locations.push_back(location);
				return ILLabel(idx);
			}

			void SetLocation(ILLabel id, Instruction* location)
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

			Instruction* GetLocation(ILLabel id) const
			{
				return locations[id.value];
			}
		};
	}
}

#endif