#ifndef DECLARATION_ANALYZER_HPP
#define DECLARATION_ANALYZER_HPP

#include "sub_analyzer.hpp"
#include <unordered_set>

namespace HXSL
{
	class HXSLDeclarationAnalyzer : public HXSLSubAnalyzer
	{
		virtual bool CanAnalyze(HXSLNode* node) override
		{
			static const std::unordered_set<HXSLNodeType> allowedTypes = { HXSLNodeType_Struct, HXSLNodeType_Function, HXSLNodeType_Field };
			return node->IsAnyTypeOf(allowedTypes);
		}

		virtual HXSLTraversalBehavior Analyze(HXSLAnalyzer& analyzer, HXSLNode* node, HXSLCompilation* compilation) override;
	};
}

#endif