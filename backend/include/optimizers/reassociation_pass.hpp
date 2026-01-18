#ifndef REASSOCIATION_PASS_HPP
#define REASSOCIATION_PASS_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ReassociationPass : public ILOptimizerPass, CFGVisitor<>
		{
			dense_map<ILVarId, ResultInstr*> definitions;
			void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

		public:
			ReassociationPass(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
			{
			}
			std::string GetName() override { return "ReassociationPass"; }

			OptimizerPassResult Run() override
			{
				changed = false;
				definitions.clear();
				Traverse();
				return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
			}
		};
	}
}

#endif