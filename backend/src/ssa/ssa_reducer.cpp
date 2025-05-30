#include "ssa/ssa_reducer.hpp"

namespace HXSL
{
	namespace Backend
	{
		void SSAReducer::TryClearVersion(ILVarId& op)
		{
			auto it = phiMap.find(op);
			if (it != phiMap.end())
			{
				op = it->second;
			}
		}

		void SSAReducer::TryClearVersion(Operand* op)
		{
			auto var = dyn_cast<Variable>(op);
			if (!var) return;
			TryClearVersion(var->varId);
		}

		void SSAReducer::Visit(size_t index, BasicBlock& node, EmptyCFGContext& context)
		{
			lastUseIndex.clear();

			for (auto& instr : node)
			{
				if (isa<PhiInstr>(&instr))
				{
					DiscardInstr(instr);
					continue;
				}

				for (auto& op : instr.GetOperands())
				{
					TryClearVersion(op);
				}

				if (auto res = dyn_cast<ResultInstr>(&instr))
				{
					TryClearVersion(res->GetResult());
				}

				Prepare(instr);
			}

			for (auto& p : seenVars)
			{
				ILVarId varId = p;
				if (!IsTempVar(varId)) continue;

				auto type = metadata.GetTempVar(varId).typeId;
				MarkAsFree(type, varId);
			}

			for (auto& instr : node)
			{
				RemapOperandsAndResult(instr);
			}

			DiscardMarkedInstructs(node);
		}

		void SSAReducer::Reduce()
		{
			for (auto& phi : metadata.phiNodes)
			{
				for (auto& p : phi->GetOperands())
				{
					phiMap.insert({ cast<Variable>(p)->varId, phi->GetResult() });
				}
			}

			Traverse();
		}
	}
}