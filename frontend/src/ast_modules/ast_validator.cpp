#include "ast_validator.hpp"

namespace HXSL
{
	TraversalBehavior ASTValidator::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		if (!node->GetParent() && !isa<CompilationUnit>(node))
		{
			auto& span = node->GetSpan();
			logger->LogFormattedInternal(LogLevel_Error, "Node {} has no parent (Line: {}, Column: {})", node->DebugName(), span.line, span.column);
			hasErrors = true;
		}
		return TraversalBehavior_Keep;
	}

	void ASTValidator::VisitClose(ASTNode* node, size_t depth)
	{
	}

	bool ASTValidator::Validate(CompilationUnit* compilation)
	{
		hasErrors = false;
		Traverse(compilation);
		return hasErrors;
	}
}
