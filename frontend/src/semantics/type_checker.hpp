#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "symbols/symbol_resolver.hpp"
#include "expression_checker.hpp"
#include "statement_checker.hpp"

namespace HXSL
{
	class TypeChecker : public ASTVisitor<EmptyDeferralContext>
	{
	private:
		SemanticAnalyzer& analyzer;
		SymbolResolver& resolver;

	public:
		TypeChecker(SemanticAnalyzer& analyzer, SymbolResolver& resolver) : analyzer(analyzer), resolver(resolver)
		{
			ExpressionCheckerRegistry::EnsureCreated();
			StatementCheckerRegistry::EnsureCreated();
		}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

		void VisitClose(ASTNode* node, size_t depth) override;

		SymbolDef* GetBoolType() const;

		SymbolDef* GetNumericType(const NumberType& type) const;

		bool TryLiteralReinterpret(Expression* expr, SymbolDef* target) const;

		bool ImplicitBinaryOperatorCheck(SymbolRef* opRef, SymbolDef* a, SymbolDef* b, Operator op, OperatorOverload*& castMatchOut);

		bool BinaryOperatorCheck(BinaryExpression* binary, Expression*& left, Expression*& right, SymbolDef*& result);

		bool BinaryCompoundOperatorCheck(CompoundAssignmentExpression* binary, Expression* left, Expression*& right, SymbolDef*& result);

		bool UnaryOperatorCheck(UnaryExpression* binary, const Expression* operand, SymbolDef*& result);

		bool CastOperatorCheck(CastExpression* cast, const SymbolDef* type, const Expression* operand, SymbolDef*& result, bool explicitCast);

		bool CastOperatorCheck(const SymbolDef* target, const SymbolDef* source, SymbolRef*& result);

		bool AreTypesCompatible(Expression*& insertPoint, SymbolDef* target, SymbolDef* source);

		bool AreTypesCompatible(Expression*& insertPointA, SymbolDef* a, Expression*& insertPointB, SymbolDef* b);

		bool IsBooleanType(Expression*& insertPoint, SymbolDef* source);

		bool IsIndexerType(Expression*& insertPoint, SymbolDef* source);

		void TypeCheckExpression(Expression* node);

		void TypeCheckStatement(ASTNode*& node);
	};
}

#endif