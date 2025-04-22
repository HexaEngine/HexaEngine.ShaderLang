#ifndef NUMBERS_H
#define NUMBERS_H

#include "text_helper.h"
#include <limits>
#include <cctype>
#include <cstdlib>
#include <sstream>

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
			float floatValue;
			double doubleValue;
		};
	};

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, Number& number, size_t& tokenLength);
}
#endif