#ifndef DEAD_CODE_ELIMINATOR_HPP
#define DEAD_CODE_ELIMINATOR_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class DeadCodeEliminator : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		std::unordered_set<ILVarId> usedVars;
		std::unordered_set<ILVarId> deadVars;

		void ProcessOperand(ILOperand& op);

		void ProcessInstr(ILInstruction& instr, size_t idx, bool protectedInstr);

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override
		{
		}

		void VisitClose(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		DeadCodeEliminator(ILMetadata& metadata, ControlFlowGraph& cfg) : ILOptimizerPass(metadata), CFGVisitor(cfg)
		{
		}

		OptimizerPassResult Run() override
		{
			changed = false;
			usedVars.clear();
			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		};
	};
}

#endif