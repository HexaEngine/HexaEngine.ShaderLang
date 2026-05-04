#ifndef DEBUG_VISITOR_HPP
#define DEBUG_VISITOR_HPP

#include "pch/ast_analyzers.hpp"

namespace HXSL 
{
	class DebugVisitor : public ASTVisitor<EmptyDeferralContext>
	{
		size_t size = 0;
		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			size += sizeof(*node);
			std::string indentation(depth * 2, ' ');
			auto& span = node->GetSpan();
			std::cout << indentation << node->DebugName() << " (Line: " << span.line << " Column: " << span.column << ")" << std::endl;
			return TraversalBehavior_Keep;
		}
	};
}

#endif