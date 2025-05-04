#ifndef ENDIAN_UTILS_HPP
#define ENDIAN_UTILS_HPP

#include <stdio.h>
#include <stdint.h>

namespace EndianUtils
{
	inline bool IsLittleEndian()
	{
		const int value{ 0x01 };
		const void* address{ static_cast<const void*>(&value) };
		const unsigned char* least_significant_address{ static_cast<const unsigned char*>(address) };

		return (*least_significant_address == 0x01);
	}

	template <typename T>
	T ReverseEndianness(T value)
	{
		static_assert(std::is_integral<T>::value, "Integral type required.");

		if constexpr (sizeof(T) == 2)
		{
			return static_cast<T>(
				((value >> 8) & 0xFF) |
				((value & 0xFF) << 8)
				);
		}
		else if constexpr (sizeof(T) == 4)
		{
			return static_cast<T>(
				((value >> 24) & 0xFF) |
				((value >> 8) & 0xFF00) |
				((value << 8) & 0xFF0000) |
				((value << 24) & 0xFF000000)
				);
		}
		else if constexpr (sizeof(T) == 8)
		{
			return static_cast<T>(
				((value >> 56) & 0xFF) |
				((value >> 40) & 0xFF00) |
				((value >> 24) & 0xFF0000) |
				((value >> 8) & 0xFF000000) |
				((value << 8) & 0xFF00000000) |
				((value << 24) & 0xFF0000000000) |
				((value << 40) & 0xFF000000000000) |
				((value << 56) & 0xFF00000000000000)
				);
		}

		return value;
	}

	template <typename T>
	T ToLittleEndian(T value)
	{
		static_assert(std::is_integral<T>::value, "Integral type required.");
		if (!IsLittleEndian())
		{
			return ReverseEndianness(value);
		}
		return value;
	}

	template <typename T>
	T FromLittleEndian(T value)
	{
		static_assert(std::is_integral<T>::value, "Integral type required.");
		return ToLittleEndian(value);
	}
}

#endif