#include "sub_analyzer_registry.hpp"

namespace HXSL
{
	std::vector<std::unique_ptr<SubAnalyzer>> SubAnalyzerRegistry::analyzers;
	std::once_flag SubAnalyzerRegistry::initFlag;

	void SubAnalyzerRegistry::EnsureCreated()
	{
		std::call_once(initFlag, []()
			{
				Register<DeclarationAnalyzer>();
			});
	}
}