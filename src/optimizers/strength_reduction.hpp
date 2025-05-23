#ifndef STRENGTH_REDUCTION_HPP
#define STRENGTH_REDUCTION_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class StrengthReduction : public CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context);

	public:
		StrengthReduction(ControlFlowGraph& cfg, ILMetadata& metadata) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}
	};
}

#endif