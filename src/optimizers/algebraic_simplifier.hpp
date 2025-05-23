#ifndef ALGEBRAIC_SIMPLIFIER_HPP
#define ALGEBRAIC_SIMPLIFIER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class AlgebraicSimplifier : public CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		bool changed = false;
	public:
		AlgebraicSimplifier(ControlFlowGraph& graph, ILMetadata& metadata) : ILMutatorBase(metadata), CFGVisitor(graph)
		{
		}

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

		bool HasChanged() const { return changed; }
	};
}

#endif