#ifndef AST_PRUNER_HPP
#define AST_PRUNER_HPP

#include "pch/ast_analyzers.hpp"

namespace HXSL
{
	class CompilationUnit : public Container
	{
		std::vector<std::unique_ptr<SymbolDef>> miscDefs;
	public:
		CompilationUnit() : Container({}, NodeType_Compilation), ASTNode({}, NodeType_Compilation) {}
	};

	class AstFlattener : public Visitor<EmptyDeferralContext>
	{
		CompilationUnit* compilationUnit;

	public:
		AstFlattener(CompilationUnit* compilationUnit) : compilationUnit(compilationUnit) {}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;
	};

	class AstPruner
	{
	public:
		AstPruner()
		{
		}

		std::unique_ptr<CompilationUnit> Flatten(Compilation* compilation);
	};
}

#endif