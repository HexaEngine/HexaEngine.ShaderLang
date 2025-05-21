#ifndef DECLARATION_ANALYZER_HPP
#define DECLARATION_ANALYZER_HPP

#include "sub_analyzer.hpp"

namespace HXSL
{
	class DeclarationAnalyzer : public SubAnalyzer
	{
		virtual bool CanAnalyze(ASTNode* node) override
		{
			static const std::unordered_set<NodeType> allowedTypes = { NodeType_Struct, NodeType_FunctionOverload, NodeType_OperatorOverload, NodeType_Field };
			return node->IsAnyTypeOf(allowedTypes);
		}

		TraversalBehavior Analyze(SemanticAnalyzer& analyzer, ASTNode* node, Compilation* compilation) override;
	};
}

#endif