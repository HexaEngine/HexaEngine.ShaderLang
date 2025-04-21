#include "numbers.h"

namespace HXSL
{
	static const std::unordered_set<char> postfixes = { 'f', 'd', 'u', 'l', 'h', 'F', 'D', 'U', 'L', 'H' };

	static bool IsValidNumberChar(char c)
	{
		return std::isdigit(c) || c == '.' || postfixes.find(c) != postfixes.end() || c == '_';
	}

	static HXSLNumberType Classify(long long value)
	{
		if (value >= std::numeric_limits<signed char>::min() && value <= std::numeric_limits<signed char>::max())
			return HXSLNumberType_SByte;
		if (value >= std::numeric_limits<short>::min() && value <= std::numeric_limits<short>::max())
			return HXSLNumberType_Short;
		if (value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
			return HXSLNumberType_Int;
		return HXSLNumberType_LongLong;
	}

	static HXSLNumberType Classify(unsigned long long value)
	{
		if (value <= std::numeric_limits<unsigned char>::max())
			return HXSLNumberType_UShort;
		if (value <= std::numeric_limits<unsigned int>::max())
			return HXSLNumberType_UInt;
		return HXSLNumberType_ULongLong;
	}

	static HXSLNumberType MakeUnsigned(HXSLNumberType type)
	{
		switch (type)
		{
		case HXSLNumberType_LongLong:
			return HXSLNumberType_ULongLong;
		case HXSLNumberType_Int:
			return HXSLNumberType_UInt;
		case HXSLNumberType_Short:
			return HXSLNumberType_UShort;
		case HXSLNumberType_SByte:
			return HXSLNumberType_Char;
		default:
			return type;
		}
	}

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, HXSLNumber& number, size_t& tokenLength)
	{
		size_t start = position;
		if (isHex || isBinary)
			position += 2;

		size_t startClean = position;
		position = TextHelper::FindEnd(position, text, length, IsValidNumberChar);

		bool isFloat = false;
		bool isUnsigned = false;
		HXSLNumberType type = HXSLNumberType_Unknown;
		size_t innerEnd = position - 1;

		if (innerEnd >= 0 && postfixes.find(text[innerEnd]) != postfixes.end())
		{
			char current = toupper(text[innerEnd]);
			switch (current)
			{
			case 'L':
				type = (type == HXSLNumberType_LongLong) ? HXSLNumberType_LongLong : HXSLNumberType_Int;
				break;
			case 'U':
				type = MakeUnsigned(type);
				isUnsigned = true;
				break;
			case 'F':
				type = HXSLNumberType_Float;
				isFloat = true;
				break;
			case 'D':
				type = HXSLNumberType_Double;
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

		if (isHex && isBinary || isFloat && (isHex || isBinary || type == HXSLNumberType_LongLong || isUnsigned))
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
			number.Kind = HXSLNumberType_ULongLong;
			number.ulongLongValue = value;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else if (isBinary)
		{
			unsigned long long value = std::stoull(numberSpan, &converted, 2);
			number.Kind = HXSLNumberType_ULongLong;
			number.ulongLongValue = value;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else if (isFloat)
		{
			if (type == HXSLNumberType_Float || type == HXSLNumberType_Unknown)
			{
				float result = std::stof(numberSpan, &converted);
				number.Kind = HXSLNumberType_Float;
				number.floatValue = result;
				tokenLength = position - start;
				return converted == spanLength;
			}
			else if (type == HXSLNumberType_Double)
			{
				double result = std::stod(numberSpan, &converted);
				number.Kind = HXSLNumberType_Double;
				number.doubleValue = result;
				tokenLength = position - start;
				return converted == spanLength;
			}
		}
		else if (isSigned)
		{
			long long result = std::stoll(numberSpan, &converted);
			type = type == HXSLNumberType_Unknown ? Classify(result) : type;
			number.Kind = type;
			number.longLongValue = result;
			tokenLength = position - start;
			return converted == spanLength;
		}
		else
		{
			unsigned long long result = std::stoull(numberSpan, &converted);
			type = type == HXSLNumberType_Unknown ? Classify(result) : type;
			number.Kind = type;
			number.longLongValue = result;
			tokenLength = position - start;
			return converted == spanLength;
		}

		return false;
	}
}