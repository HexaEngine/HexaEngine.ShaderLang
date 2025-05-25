#ifndef CONSTANT_FOLDER_HPP
#define CONSTANT_FOLDER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class ConstantFolder : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		std::unordered_map<ILVarId, Number> constants;
		std::unordered_map<ILVarId, ILVarId> varToVar;

		void TryFoldOperand(ILOperand& op);

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		ConstantFolder(ILMetadata& metadata, ControlFlowGraph& cfg) : ILOptimizerPass(metadata), CFGVisitor(cfg)
		{
		}

		OptimizerPassResult Run() override
		{
			changed = false;
			constants.clear();
			varToVar.clear();
			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		}
	};
}

#endif