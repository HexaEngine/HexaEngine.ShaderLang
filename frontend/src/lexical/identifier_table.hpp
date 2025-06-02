#ifndef IDENTIFIER_TABLE_HPP
#define IDENTIFIER_TABLE_HPP

#include "pch/std.hpp"
#include "utils/span.hpp"
#include "utils/dense_map.hpp"
#include "utils/bump_allocator.hpp"

namespace HXSL
{
	struct IdentifierInfo
	{
		StringSpan name;

		size_t hash() const { return name.hash(); }

		inline bool operator==(const IdentifierInfo& other) const { return name == other.name; }
		inline bool operator!=(const IdentifierInfo& other) const { return !(*this == other); }
	};

	class IdentifierTable
	{
		BumpAllocator allocator;
		dense_map<StringSpan, IdentifierInfo*> map;

	public:
		IdentifierInfo* Get(const StringSpan& name)
		{
			auto it = map.find(name);
			if (it != map.end())
			{
				return it->second;
			}

			auto info = allocator.Alloc<IdentifierInfo>();

			auto p = reinterpret_cast<char*>(allocator.Alloc(name.size() * sizeof(char), alignof(char)));
			std::memcpy(p, name.data(), name.size() * sizeof(char));
			info->name = StringSpan(p, name.size());

			map.insert({ info->name, info });
			return info;
		}
	};
}

#endif