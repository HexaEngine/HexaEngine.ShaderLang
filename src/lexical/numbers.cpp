#include "numbers.h"

namespace HXSL 
{
	static const std::unordered_set<char> postfixes = { 'f', 'd', 'u', 'l', 'h', 'F', 'D', 'U', 'L', 'H' };

	static bool IsValidNumberChar(char c)
	{
		return std::isdigit(c) || c == '.' || postfixes.find(c) != postfixes.end() || c == '_';
	}

	static NumberType Classify(long long value)
	{
		if (value >= std::numeric_limits<signed char>::min() && value <= std::numeric_limits<signed char>::max())
			return NumberType_SByte;
		if (value >= std::numeric_limits<short>::min() && value <= std::numeric_limits<short>::max())
			return NumberType_Short;
		if (value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
			return NumberType_Int;
		return NumberType_LongLong;
	}

	static NumberType Classify(unsigned long long value)
	{
		if (value <= std::numeric_limits<unsigned char>::max())
			return NumberType_UShort;
		if (value <= std::numeric_limits<unsigned int>::max())
			return NumberType_UInt;
		return NumberType_ULongLong;
	}

	static NumberType MakeUnsigned(NumberType type)
	{
		switch (type)
		{
		case NumberType_LongLong:
			return NumberType_ULongLong;
		case NumberType_Int:
			return NumberType_UInt;
		case NumberType_Short:
			return NumberType_UShort;
		case NumberType_SByte:
			return NumberType_Char;
		default:
			return type;
		}
	}

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, Number& number, size_t& tokenLength)
	{
		size_t start = position;
		if (isHex || isBinary)
			position += 2;

		size_t startClean = position;
		position = TextHelper::FindEnd(position, text, length, IsValidNumberChar);

		bool isFloat = false;
		bool isUnsigned = false;
		NumberType type = NumberType_Unknown;
		size_t innerEnd = position - 1;

		if (innerEnd >= 0 && postfixes.find(text[innerEnd]) != postfixes.end())
		{
			char current = toupper(text[innerEnd]);
			switch (current)
			{
			case 'L':
				type = (type == NumberType_LongLong) ? NumberType_LongLong : NumberType_Int;
				break;
			case 'U':
				type = MakeUnsigned(type);
				isUnsigned = true;
				break;
			case 'F':
				type = NumberType_Float;
				isFloat = true;
				break;
			case 'D':
				type = NumberType_Double;
				isFloat = true;
				break;

			case 'H':
				isHex = true;
				break;

			case 'B':
				isBinary = true;
				break;
			}
			innerEnd--;
		}

		if (isHex && isBinary || isFloat && (isHex || isBinary || type == NumberType_LongLong || isUnsigned))
		{
			return false;
		}

		size_t spanLength = innerEnd + 1 - startClean;
		std::string numberSpan(text + startClean, spanLength);

		int bufferIndex = 0;
		for (int i = 0; i < spanLength; i++)
		{
			auto ch = numberSpan[i];

			if (!isHex && std::isalpha(ch))
			{
				return false;
			}

			if (isBinary && !(ch == '0' || ch == '1' || ch == '_'))
			{
				return false;
			}

			if (ch == '.') isFloat = true;

			if (ch != '_')
			{
				numberSpan[bufferIndex++] = ch;
			}
		}

		numberSpan.resize(bufferIndex);
		spanLength = bufferIndex;

		size_t converted;
		if (isHex)
		{
			unsigned long long value = std::stoull(numberSpan, &converted, 16);
			number.Kind = NumberType_ULongLong;
			number.ulongLongValue = value;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else if (isBinary)
		{
			unsigned long long value = std::stoull(numberSpan, &converted, 2);
			number.Kind = NumberType_ULongLong;
			number.ulongLongValue = value;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else if (isFloat)
		{
			if (type == NumberType_Float || type == NumberType_Unknown)
			{
				float result = std::stof(numberSpan, &converted);
				number.Kind = NumberType_Float;
				number.floatValue = result;
				tokenLength = position - start;
				return converted == spanLength;
			}
			else if (type == NumberType_Double)
			{
				double result = std::stod(numberSpan, &converted);
				number.Kind = NumberType_Double;
				number.doubleValue = result;
				tokenLength = position - start;
				return converted == spanLength;
			}
		}
		else if (isSigned)
		{
			long long result = std::stoll(numberSpan, &converted);
			type = type == NumberType_Unknown ? Classify(result) : type;
			number.Kind = type;
			number.longLongValue = result;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else
		{
			unsigned long long result = std::stoull(numberSpan, &converted);
			type = type == NumberType_Unknown ? Classify(result) : type;
			number.Kind = type;
			number.longLongValue = result;
			tokenLength = position - start;
			return converted == spanLength;
		}

		return false;
	}
}