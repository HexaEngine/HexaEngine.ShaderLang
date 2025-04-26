#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "symbol_resolver.hpp"
#include "expression_checker.hpp"

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
			ExpressionCheckerRegistry::EnsureCreated();
		}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

		void VisitClose(ASTNode* node, size_t depth) override;

		SymbolDef* GetNumericType(const NumberType& type) const;

		bool BinaryOperatorCheck(BinaryExpression* binary, const Expression* left, const Expression* right, SymbolDef*& result);

		bool UnaryOperatorCheck(UnaryExpression* binary, const Expression* operand, SymbolDef*& result);

		bool CastOperatorCheck(CastExpression* unary, const Expression* typeExpr, const Expression* operand, SymbolDef*& result, bool explicitCast);

		bool AreTypesCompatible(SymbolDef* a, SymbolDef* b);

		bool IsBooleanType(SymbolDef* a);

		void TypeCheckExpression(Expression* node);

		void TypeCheckStatement(ASTNode*& node);
	};
}

#endif