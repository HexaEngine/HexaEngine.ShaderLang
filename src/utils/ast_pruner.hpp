#ifndef AST_PRUNER_HPP
#define AST_PRUNER_HPP

#include "pch/ast_analyzers.hpp"
#include "ast_ilgen.hpp"

namespace HXSL
{
	class ASTPruner : Visitor<EmptyDeferralContext>
	{
		LowerCompilationUnit* compilationUnit;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

	public:
		void Prune(LowerCompilationUnit* compilation);
	};
}

#endif