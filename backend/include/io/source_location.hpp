#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include "pch/std.hpp"

namespace HXSL
{
	using SourceFileID = uint32_t;
	using SourceOffset = uint32_t;

	constexpr SourceFileID INVALID_SOURCE_ID = std::numeric_limits<SourceFileID>::max();

	struct SourceLocation
	{
		SourceFileID sourceId;
		SourceOffset offset;
	};

	struct SourceSpan
	{
		SourceLocation location;
		SourceOffset length;
	};
}

#endif