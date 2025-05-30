#ifndef CONSTANT_FOLDER_HPP
#define CONSTANT_FOLDER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ConstantFolder : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
		{
			std::unordered_map<ILVarId, Number> constants;
			std::unordered_map<ILVarId, ILVarId> varToVar;

			void TryFoldOperand(Operand*& op);

			void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

		public:
			ConstantFolder(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
			{
			}

			std::string GetName() override { return "ConstantFolder"; }

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
}

#endif