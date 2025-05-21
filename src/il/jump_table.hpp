#ifndef JUMP_TABLE_HPP
#define JUMP_TABLE_HPP

#include "pch/ast.hpp"

namespace HXSL
{
	constexpr uint64_t INVALID_JUMP_LOCATION = -1;

	struct JumpTable
	{
		std::vector<size_t> locations;

		uint64_t Allocate(size_t location = INVALID_JUMP_LOCATION)
		{
			auto idx = locations.size();
			locations.push_back(location);
			return idx;
		}

		void SetLocation(uint64_t id, size_t location)
		{
			locations[id] = location;
		}

		size_t GetLocation(uint64_t id) const
		{
			return locations[id];
		}

		void Relocate(const std::vector<ILDiff>& diffs)
		{
			size_t diffCount = diffs.size();
			int64_t offset = 0;
			size_t diffIndex = 0;
			for (auto& loc : locations)
			{
				while (diffIndex < diffCount)
				{
					auto& diff = diffs[diffIndex];
					if (diff.start < loc)
					{
						offset += diff.diff;
						diffIndex++;
						continue;
					}
					break;
				}

				loc += offset;
			}
		}
	};
}

#endif