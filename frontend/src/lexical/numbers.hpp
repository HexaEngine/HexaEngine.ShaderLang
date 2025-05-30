#ifndef NUMBERS_HPP
#define NUMBERS_HPP

#include "core/number.hpp"

namespace HXSL
{
	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, Number& number, size_t& tokenLength);
}
#endif