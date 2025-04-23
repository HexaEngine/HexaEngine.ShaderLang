#ifndef SUB_ANALYZER_HPP
#define SUB_ANALYZER_HPP

#include "analyzer.hpp"
#include "ast.hpp"

namespace HXSL
{
	class SubAnalyzer
	{
	protected:
		SubAnalyzer()
		{
		}

	public:
		virtual bool CanAnalyze(ASTNode* node) = 0;

		virtual TraversalBehavior Analyze(Analyzer& analyzer, ASTNode* node, Compilation* compilation) = 0;

		TraversalBehavior TryAnalyze(Analyzer& analyzer, ASTNode* node, Compilation* compilation)
		{
			if (CanAnalyze(node))
			{
				return Analyze(analyzer, node, compilation);
			}
			return TraversalBehavior_AnalyzerSkip;
		}
	};
}

#endif