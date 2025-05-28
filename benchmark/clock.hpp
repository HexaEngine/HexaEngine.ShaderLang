#ifndef CLOCK_HPP
#define CLOCK_HPP

#include "pch/std.hpp"

#define NOMINMAX
#include <Windows.h>

namespace Bench
{
	struct clock
	{
		inline static uint64_t now()
		{
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			return now.QuadPart;
		}
	};

	struct duration
	{
		inline static uint64_t freqency()
		{
			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);
			return freq.QuadPart;
		}
	};
}

#endif