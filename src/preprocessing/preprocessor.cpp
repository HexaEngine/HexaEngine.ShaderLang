#include "preprocessor.hpp"
#include "generated/localization.hpp"

namespace HXSL
{
	int Preprocessor::Transform(const Token& current, TokenStream& stream, Token& outToken)
	{
		if (current.isNewLine())
		{
			return 2;
		}
		if (current.isDelimiterOf('#'))
		{
			//stream.LogFormatted();
			stream.Advance();
			return 0;
		}

		if (!current.isKeyword())
		{
			return 0;
		}

		auto keyword = current.asKeyword();

		switch (keyword)
		{
		case Keyword_PrepDefine:
		{
			stream.Advance();
			TextSpan name;
			stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER);
			while (!stream.Current().isNewLine())
			{
				stream.TryAdvance();
			}
			stream.TryAdvance();
		}
		break;
		case Keyword_PrepIf:
			break;
		case Keyword_PrepElif:
			break;
		case Keyword_PrepElse:
			break;
		case Keyword_PrepEndif:
			break;
		case Keyword_PrepInclude:
			break;
		case Keyword_PrepError:
			break;
		case Keyword_PrepPragma:
			break;
		default:
			return 0;
			break;
		}

		return 0;
	}
}