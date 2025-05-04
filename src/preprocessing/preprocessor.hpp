#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "lexical/token_stream.h"
#include "preprocessor_stream.hpp"
#include "utils/span.h"
#include <unordered_map>

namespace HXSL
{
	struct PreprocessorSymbol
	{
		std::unique_ptr<std::string> name;
		std::vector<Token> tokens;
	};

	class Preprocessor : public TokenTransformer
	{
		std::unordered_map<StringSpan, PreprocessorSymbol, StringSpanHash, StringSpanEqual> symbolTable;

		void TryExpand(const TextSpan& span);
	public:
		int Transform(const Token& current, TokenStream& stream, Token& outToken) override;
	};
}

#endif