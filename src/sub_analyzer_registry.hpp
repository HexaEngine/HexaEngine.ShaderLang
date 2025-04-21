#ifndef SUB_ANALYZER_REGISTRY_HPP
#define SUB_ANALYZER_REGISTRY_HPP

#include "config.h"
#include "analyzer.hpp"
#include "sub_analyzer.hpp"

namespace HXSL
{
	class HXSLSubAnalyzerRegistry
	{
	private:
		static std::vector<std::unique_ptr<HXSLSubAnalyzer>> analyzers;

	public:
		static HXSLTraversalBehavior TryAnalyze(HXSLAnalyzer& analyzer, HXSLNode* node, HXSLCompilation* compilation)
		{
			for (auto& subAnalyzer : analyzers)
			{
				HXSLTraversalBehavior result = subAnalyzer->TryAnalyze(analyzer, node, compilation);
				if (result != HXSLTraversalBehavior_AnalyzerSkip)
				{
					return result;
				}
			}
#ifdef ALLOW_PARTIAL_ANALYSIS
			return HXSLTraversalBehavior_Keep;
#else
			return HXSLTraversalBehavior_Break;
#endif
		}

		template <typename SubAnalyzerType>
		static typename std::enable_if<std::is_base_of<HXSLSubAnalyzer, SubAnalyzerType>::value>::type
			Register()
		{
			analyzers.push_back(std::make_unique<SubAnalyzerType>());
		}
	};
}
#endif