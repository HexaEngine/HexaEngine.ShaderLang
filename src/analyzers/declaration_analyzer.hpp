#ifndef DECLARATION_ANALYZER_HPP
#define DECLARATION_ANALYZER_HPP

#include "sub_analyzer.hpp"
#include <unordered_set>
#include <memory>

namespace HXSL
{
	class HXSLDeclarationAnalyzer : public HXSLSubAnalyzer
	{
		virtual bool CanAnalyze(ASTNode* node) override
		{
			static const std::unordered_set<HXSLNodeType> allowedTypes = { HXSLNodeType_Struct, HXSLNodeType_Function, HXSLNodeType_Field };
			return node->IsAnyTypeOf(allowedTypes);
		}

		virtual HXSLTraversalBehavior Analyze(HXSLAnalyzer& analyzer, ASTNode* node, Compilation* compilation) override;
	};
}

#endif