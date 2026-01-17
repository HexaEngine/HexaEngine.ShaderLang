#ifndef GLOBAL_VALUE_NUMBERING_HPP
#define GLOBAL_VALUE_NUMBERING_HPP

#include "il_optimizer_pass.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	namespace Backend
	{
		class GlobalValueNumbering : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
		{
			std::unordered_set<ResultInstr*, InstructionPtrHash, InstructionPtrEquals> subExpressions;
			dense_map<ILVarId, ILVarId> map;

			void TryMapOperand(Operand*& op);

			void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

		public:
			GlobalValueNumbering(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
			{
			}

			std::string GetName() override { return "GlobalValueNumbering"; }

			OptimizerPassResult Run() override
			{
				changed = false;
				subExpressions.clear();
				map.clear();
				Traverse();
				return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
			}
		};
	}
}

#endif