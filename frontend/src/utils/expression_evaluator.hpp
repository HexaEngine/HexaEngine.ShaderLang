#ifndef EXPRESSION_EVALUATOR_HPP
#define EXPRESSION_EVALUATOR_HPP

#include "pch/ast.hpp"

namespace HXSL 
{

	struct ExpressionEvaluatorContext
	{
		dense_map<SymbolDef*, Number> symbols;
	};

	struct ExpressionEvaluatorResult
	{
		bool success;
		Number value;
		dense_set<SymbolDef*> unresolved;
	};

	class ExpressionEvaluator
	{
	public:
		static ExpressionEvaluatorResult TryEvaluate(Expression* expr, ExpressionEvaluatorContext& ctx);
	};
}

#endif