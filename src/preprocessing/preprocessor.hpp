#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "lexical/token_stream.h"
#include "preprocessor_stream.hpp"
#include "utils/span.hpp"
#include <unordered_map>

namespace HXSL
{
	struct PreprocessorSymbol
	{
		std::unique_ptr<std::string> name;
		std::vector<Token> tokens;

		PreprocessorSymbol(TextSpan span) : name(std::make_unique<std::string>(span.str()))
		{
		}
	};

	class Preprocessor : public TokenTransformer
	{
		std::unordered_map<StringSpan, PreprocessorSymbol, StringSpanHash, StringSpanEqual> symbolTable;
		size_t lastIndex = 0;

		void TryExpand(const TextSpan& span);
	public:
		int Transform(Token& current, TokenStream& stream) override;
	};
}

#endif