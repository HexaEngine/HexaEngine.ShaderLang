#ifndef SUB_ANALYZER_HPP
#define SUB_ANALYZER_HPP

#include "semantic_analyzer.hpp"
#include "pch/ast.hpp"

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

		virtual TraversalBehavior Analyze(SemanticAnalyzer& analyzer, ASTNode* node, CompilationUnit* compilation) = 0;

		TraversalBehavior TryAnalyze(SemanticAnalyzer& analyzer, ASTNode* node, CompilationUnit* compilation)
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