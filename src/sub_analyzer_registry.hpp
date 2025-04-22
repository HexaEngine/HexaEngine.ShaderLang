#ifndef SUB_ANALYZER_REGISTRY_HPP
#define SUB_ANALYZER_REGISTRY_HPP

#include "config.h"
#include "analyzer.hpp"
#include "sub_analyzer.hpp"

namespace HXSL
{
	class SubAnalyzerRegistry
	{
	private:
		static std::vector<std::unique_ptr<SubAnalyzer>> analyzers;

	public:
		static TraversalBehavior TryAnalyze(Analyzer& analyzer, ASTNode* node, Compilation* compilation)
		{
			for (auto& subAnalyzer : analyzers)
			{
				TraversalBehavior result = subAnalyzer->TryAnalyze(analyzer, node, compilation);
				if (result != TraversalBehavior_AnalyzerSkip)
				{
					return result;
				}
			}
#ifdef ALLOW_PARTIAL_ANALYSIS
			return TraversalBehavior_Keep;
#else
			return TraversalBehavior_Break;
#endif
		}

		template <typename SubAnalyzerType>
		static typename std::enable_if<std::is_base_of<SubAnalyzer, SubAnalyzerType>::value>::type
			Register()
		{
			analyzers.push_back(std::make_unique<SubAnalyzerType>());
		}
	};
}
#endif