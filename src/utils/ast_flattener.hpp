#ifndef AST_FLATTENER_HPP
#define AST_FLATTENER_HPP

#include "pch/ast_analyzers.hpp"

namespace HXSL
{
	class ASTFlattener : Visitor<EmptyDeferralContext>
	{
		std::unique_ptr<LowerCompilationUnit> compilationUnit;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

	public:
		std::unique_ptr<LowerCompilationUnit> Flatten(CompilationUnit* compilation);
	};
}

#endif