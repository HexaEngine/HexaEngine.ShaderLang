#ifndef NUMBERS_H
#define NUMBERS_H

#include "utils/text_helper.h"
#include <limits>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <utils/half.hpp>

namespace HXSL
{
	enum NumberType
	{
		NumberType_Unknown,
		NumberType_Char,
		NumberType_SByte,
		NumberType_Short,
		NumberType_UShort,
		NumberType_Int,
		NumberType_UInt,
		NumberType_LongLong,
		NumberType_ULongLong,
		NumberType_Half,
		NumberType_Float,
		NumberType_Double,
	};

	struct Number
	{
		Number() : Kind(NumberType_Unknown), longLongValue(0)
		{
		}

		NumberType Kind;
		union {
			char charValue;
			signed char sbyteValue;
			short shortValue;
			unsigned short ushortValue;
			int intValue;
			unsigned int uintValue;
			long long longLongValue;
			unsigned long long ulongLongValue;
			half halfValue;
			float floatValue;
			double doubleValue;
		};

		bool IsSigned() const noexcept
		{
			switch (Kind)
			{
			case NumberType_SByte:
			case NumberType_Short:
			case NumberType_Int:
			case NumberType_LongLong:
			case NumberType_Half:
			case NumberType_Float:
			case NumberType_Double:
				return true;
			default:
				return false;
			}
		}

		bool IsNegative() const noexcept
		{
			switch (Kind)
			{
			case NumberType_SByte:
				return sbyteValue < 0;
			case NumberType_Short:
				return shortValue < 0;
			case NumberType_Int:
				return intValue < 0;
			case NumberType_LongLong:
				return longLongValue < 0;
			case NumberType_Half:
				return halfValue < 0 && !halfValue.IsNaN();
			case NumberType_Float:
				return floatValue < 0 && !std::isnan(floatValue);
			case NumberType_Double:
				return doubleValue < 0 && !std::isnan(doubleValue);
			default:
				return false;
			}
		}

		bool IsIntegral() const noexcept
		{
			switch (Kind)
			{
			case NumberType_Char:
			case NumberType_SByte:
			case NumberType_Short:
			case NumberType_UShort:
			case NumberType_Int:
			case NumberType_UInt:
			case NumberType_LongLong:
			case NumberType_ULongLong:
				return true;
			default:
				return false;
			}
		}

		size_t ToSizeT() const noexcept
		{
			switch (Kind)
			{
			case NumberType_Char:
				return static_cast<size_t>(charValue);
			case NumberType_SByte:
				return static_cast<size_t>(sbyteValue);
			case NumberType_Short:
				return static_cast<size_t>(shortValue);
			case NumberType_UShort:
				return static_cast<size_t>(ushortValue);
			case NumberType_Int:
				return static_cast<size_t>(intValue);
			case NumberType_UInt:
				return static_cast<size_t>(uintValue);
			case NumberType_LongLong:
				return static_cast<size_t>(longLongValue);
			case NumberType_ULongLong:
				return static_cast<size_t>(ulongLongValue);
			default:
				return 0;
			}
		}
	};

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, Number& number, size_t& tokenLength);
}
#endif