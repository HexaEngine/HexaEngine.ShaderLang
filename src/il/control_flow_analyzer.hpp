#ifndef CONTROL_FLOW_ANALYZER_HPP
#define CONTROL_FLOW_ANALYZER_HPP

#include "control_flow_graph.hpp"

namespace HXSL
{
	class ControlFlowAnalyzer : public CFGVisitor<EmptyCFGContext>
	{
		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;
	};
}

#endif