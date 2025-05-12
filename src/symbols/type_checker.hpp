#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "symbol_resolver.hpp"
#include "expression_checker.hpp"
#include "statement_checker.hpp"

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
			StatementCheckerRegistry::EnsureCreated();
		}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

		void VisitClose(ASTNode* node, size_t depth) override;

		SymbolDef* GetNumericType(const NumberType& type) const;

		bool ImplicitBinaryOperatorCheck(SymbolRef* opRef, SymbolDef* a, SymbolDef* b, Operator op, OperatorOverload*& castMatchOut);

		bool BinaryOperatorCheck(BinaryExpression* binary, std::unique_ptr<Expression>& left, std::unique_ptr<Expression>& right, SymbolDef*& result);

		bool BinaryCompoundOperatorCheck(CompoundAssignmentExpression* binary, const std::unique_ptr<Expression>& left, std::unique_ptr<Expression>& right, SymbolDef*& result);

		bool UnaryOperatorCheck(UnaryExpression* binary, const Expression* operand, SymbolDef*& result);

		bool CastOperatorCheck(CastExpression* cast, const SymbolDef* type, const Expression* operand, SymbolDef*& result, bool explicitCast);

		bool CastOperatorCheck(const SymbolDef* target, const SymbolDef* source, std::unique_ptr<SymbolRef>& result);

		bool AreTypesCompatible(std::unique_ptr<Expression>& insertPoint, SymbolDef* target, SymbolDef* source);

		bool AreTypesCompatible(std::unique_ptr<Expression>& insertPointA, SymbolDef* a, std::unique_ptr<Expression>& insertPointB, SymbolDef* b);

		bool IsBooleanType(std::unique_ptr<Expression>& insertPoint, SymbolDef* source);

		bool IsIndexerType(std::unique_ptr<Expression>& insertPoint, SymbolDef* source);

		void TypeCheckExpression(Expression* node);

		void TypeCheckStatement(ASTNode*& node);
	};
}

#endif