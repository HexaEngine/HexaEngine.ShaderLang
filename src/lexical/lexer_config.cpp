#include "lexer_config.hpp"
#include "lang/language.hpp"
#include <mutex>

namespace HXSL
{
	static std::once_flag initFlag;
	static std::unique_ptr<LexerConfig> mainConfig;
	static std::unique_ptr<LexerConfig> preprocessorConfig;

	static void Init()
	{
		mainConfig = std::make_unique<LexerConfig>();
		BuildKeywordTST(&mainConfig->keywords);
		BuildOperatorTST(&mainConfig->operators);
		mainConfig->delimiters = { '{', '}', '[', ']', '(', ')', ',', ';', '#', '@' };

		preprocessorConfig = std::make_unique<LexerConfig>();
		preprocessorConfig->enableNewline = true;
		preprocessorConfig->enableWhitespace = true;
		BuildKeywordTST(&preprocessorConfig->keywords);
		BuildOperatorTST(&preprocessorConfig->operators);
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