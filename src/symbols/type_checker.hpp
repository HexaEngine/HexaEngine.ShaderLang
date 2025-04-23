#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "node_visitor.hpp"
#include "symbol_resolver.hpp"

namespace HXSL
{
	class TypeChecker : public Visitor<EmptyDeferralContext>
	{
	private:
		Analyzer& analyzer;
		SymbolResolver& resolver;

	public:
		TypeChecker(Analyzer& analyzer, SymbolResolver& resolver) : analyzer(analyzer), resolver(resolver)
		{
		}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

		void VisitClose(ASTNode* node, size_t depth) override;

		SymbolDef* GetNumericType(const NumberType& type) const;

		void TypeCheckExpression(Expression* node);

		void TypeCheckStatement(ASTNode*& node);
	};
}

#endif