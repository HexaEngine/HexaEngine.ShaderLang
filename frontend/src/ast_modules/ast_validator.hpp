#ifndef AST_VALIDATOR_HPP
#define AST_VALIDATOR_HPP

#include "pch/ast.hpp"

namespace HXSL
{
	class ASTValidator : ASTVisitor<>
	{
	private:
		ILogger* logger;
		bool hasErrors;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;
		void VisitClose(ASTNode* node, size_t depth) override;

	public:
		ASTValidator(ILogger* logger) : ASTVisitor<>(), logger(logger), hasErrors(false) {}
		bool Validate(CompilationUnit* compilation);
	};
}

#endif