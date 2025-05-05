#include "preprocessor.hpp"
#include "generated/localization.hpp"

namespace HXSL
{
	int Preprocessor::Transform(Token& current, TokenStream& stream)
	{
		auto& state = stream.GetLexerState();

		if (lastIndex > state.Index)
		{
			return 0;
		}

		lastIndex = state.Index;

		if (current.isNewLine())
		{
			return 2;
		}
		if (current.isDelimiterOf('#'))
		{
			stream.LogFormatted(EXPECTED_PREP_DIRECTIVE);
			stream.Advance();
			return 0;
		}

		if (current.isIdentifier())
		{
			auto id = current.Span.str();
			auto it = symbolTable.find(id);
			if (it == symbolTable.end())
			{
				return 0;
			}
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
			stream.ExpectIdentifier(name);
			PreprocessorSymbol symbol = PreprocessorSymbol(name);
			while (!stream.Current().isNewLine())
			{
				symbol.tokens.push_back(stream.Current());
				if (!stream.TryAdvance())
				{
					break;
				}
			}

			StringSpan nameSpan = *symbol.name.get();
			symbolTable.insert(std::make_pair(nameSpan, std::move(symbol)));
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