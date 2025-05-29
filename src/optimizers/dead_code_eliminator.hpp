#ifndef DEAD_CODE_ELIMINATOR_HPP
#define DEAD_CODE_ELIMINATOR_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class DeadCodeEliminator : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		std::unordered_set<ILVarId> usedVars;
		std::unordered_set<ILVarId> deadVars;

		void ProcessOperand(Operand* op);

		void ProcessInstr(Instruction& instr, bool protectedInstr);

		void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override
		{
		}

		void VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

	public:
		DeadCodeEliminator(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
		{
		}

		std::string GetName() override { return "DeadCodeEliminator"; }

		OptimizerPassResult Run() override;
	};
}

#endif