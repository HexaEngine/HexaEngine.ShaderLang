#ifndef SUB_ANALYZER_HPP
#define SUB_ANALYZER_HPP

#include "analyzer.hpp"
#include "nodes.hpp"
#include "compilation.hpp"

namespace HXSL
{
	class HXSLSubAnalyzer
	{
	protected:
		HXSLSubAnalyzer()
		{
		}

	public:
		virtual bool CanAnalyze(ASTNode* node) = 0;

		virtual HXSLTraversalBehavior Analyze(HXSLAnalyzer& analyzer, ASTNode* node, Compilation* compilation) = 0;

		HXSLTraversalBehavior TryAnalyze(HXSLAnalyzer& analyzer, ASTNode* node, Compilation* compilation)
		{
			if (CanAnalyze(node))
			{
				return Analyze(analyzer, node, compilation);
			}
			return HXSLTraversalBehavior_AnalyzerSkip;
		}
	};
}

#endif