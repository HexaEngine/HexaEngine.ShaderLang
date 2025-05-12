#ifndef LEXER_CONFIG_HPP
#define LEXER_CONFIG_HPP

#include "utils/tst.hpp"

#include <unordered_set>

namespace HXSL
{
	class LexerConfig
	{
	public:
		bool enableNewline;
		bool enableWhitespace;
		std::unordered_set<char> delimiters;
		TernarySearchTreeDictionary<int> keywords;
		TernarySearchTreeDictionary<int> operators;

		LexerConfig() : enableNewline(false), enableWhitespace(false)
		{
		}

		LexerConfig(bool enableNewline, bool enableWhitespace, bool specialParseTreatIdentiferAsLiteral, const std::unordered_set<char>& delimiters, const TernarySearchTreeDictionary<int>& keywords, const TernarySearchTreeDictionary<int>& operators)
			: enableNewline(enableNewline), enableWhitespace(enableWhitespace), delimiters(delimiters), keywords(keywords), operators(operators)
		{
		}
	};

	class HXSLLexerConfig
	{
	public:
		static LexerConfig* Instance();
		static LexerConfig* InstancePreprocess();

		HXSLLexerConfig() = delete;
		~HXSLLexerConfig() = delete;
		HXSLLexerConfig(const HXSLLexerConfig&) = delete;
		HXSLLexerConfig& operator=(const HXSLLexerConfig&) = delete;
	};
}

#endif