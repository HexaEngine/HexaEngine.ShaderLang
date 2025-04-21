#ifndef NUMBERS_H
#define NUMBERS_H

#include "text_helper.h"
#include <limits>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace HXSL
{
	enum HXSLNumberType
	{
		HXSLNumberType_Unknown,
		HXSLNumberType_Char,
		HXSLNumberType_SByte,
		HXSLNumberType_Short,
		HXSLNumberType_UShort,
		HXSLNumberType_Int,
		HXSLNumberType_UInt,
		HXSLNumberType_LongLong,
		HXSLNumberType_ULongLong,
		HXSLNumberType_Half,
		HXSLNumberType_Float,
		HXSLNumberType_Double,
	};

	struct HXSLNumber
	{
		HXSLNumber() : Kind(HXSLNumberType_Unknown), longLongValue(0)
		{
		}

		HXSLNumberType Kind;
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

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, HXSLNumber& number, size_t& tokenLength);
}
#endif