#include "lexer_config.hpp"
#include "lang/language.hpp"
#include "pch/std.hpp"

namespace HXSL
{
	static std::once_flag initFlag;
	static std::unique_ptr<LexerConfig> mainConfig;
	static std::unique_ptr<LexerConfig> preprocessorConfig;

	static void Init()
	{
		mainConfig = std::make_unique<LexerConfig>();
		BuildKeywordRadix(mainConfig->keywords);
		BuildOperatorRadix(mainConfig->operators);
		mainConfig->delimiters = { '{', '}', '[', ']', '(', ')', ',', ';', '#', '@' };

		preprocessorConfig = std::make_unique<LexerConfig>();
		preprocessorConfig->enableNewline = true;
		preprocessorConfig->enableWhitespace = true;
		BuildKeywordRadix(preprocessorConfig->keywords);
		BuildOperatorRadix(preprocessorConfig->operators);
		preprocessorConfig->delimiters = { '{', '}', '[', ']', '(', ')', ',', ';', '#', '@' };
	}

	LexerConfig* HXSLLexerConfig::Instance()
	{
		std::call_once(initFlag, Init);
		return mainConfig.get();
	}

	LexerConfig* HXSLLexerConfig::InstancePreprocess()
	{
		std::call_once(initFlag, Init);
		return preprocessorConfig.get();
	}
}