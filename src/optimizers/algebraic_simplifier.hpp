#ifndef ALGEBRAIC_SIMPLIFIER_HPP
#define ALGEBRAIC_SIMPLIFIER_HPP

#include "il/control_flow_graph.hpp"
#include "il/il_mutator_base.hpp"

namespace HXSL
{
	class AlgebraicSimplifier : public CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		bool changed = false;
	public:
		AlgebraicSimplifier(ILBuilder& builder, ControlFlowGraph& graph) : ILMutatorBase(builder), CFGVisitor(graph)
		{
		}

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

		bool HasChanged() const { return changed; }
	};
}

#endif