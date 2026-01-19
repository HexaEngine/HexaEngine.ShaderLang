#include "il/rpo_merger.hpp"

namespace HXSL
{
	namespace Backend
	{
		void RPOMerger::VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context)
		{
			blocksSorted.push_back(&node);
		}

		void RPOMerger::Merge(BumpAllocator& allocator, ILMetadata& srcMetadata, ILMetadata& dstMetadata, ILContainer& container, JumpTable& jumpTable)
		{
			blocksSorted.clear();
			TraverseDFS();

			std::reverse(blocksSorted.begin(), blocksSorted.end());
			
			// Note: This assumes SSA form has already been deconstructed.
			// The variable mapping performs two tasks:
			// 1. Compacts variable IDs for better memory efficiency
			// 2. Strips orphan metadata (e.g., dead types from optimizer-removed variables)
			// This ensures the final metadata contains only actively used definitions.

			container.clear();
			dense_map<ILVarId, ILVarId> varMap;
			for (auto& block : blocksSorted)
			{
				auto& instructions = block->GetInstructions();
				if (instructions.empty())
				{
					continue;
				}

				Instruction* firstInstr = nullptr;
				for (auto& instr : instructions)
				{
					auto* clonedInstr = instr.Clone(allocator);
					clonedInstr->SetParent(nullptr);
					if (!firstInstr)
					{
						firstInstr = clonedInstr;
					}

					if (auto resultInstr = dyn_cast<ResultInstr>(clonedInstr))
					{
						auto oldVarId = resultInstr->GetResult();
						auto it = varMap.find(oldVarId);
						if (it != varMap.end())
						{
							resultInstr->SetResult(it->second);
						}
						else
						{
							auto& newVar = dstMetadata.CloneVar(oldVarId, srcMetadata.GetVar(oldVarId));
							auto newVarId = newVar.id;
							varMap.insert({ oldVarId, newVarId });
							resultInstr->SetResult(newVarId);
						}
					}

					if (auto callInstr = dyn_cast<CallInstr>(clonedInstr))
					{
						auto func = callInstr->GetFunction();
						auto newFunc = dstMetadata.RegFunc(func->funcId->func);
						func->funcId = newFunc;
						newFunc->callSites.push_back(callInstr);
					}

					for (auto& operand : clonedInstr->GetOperands())
					{
						if (auto var = dyn_cast<Variable>(operand))
						{
							auto it = varMap.find(var->varId);
							HXSL_ASSERT(it != varMap.end(), "Variable is not mapped! This should never happen.");
							var->varId = it->second;
						}
					}

					container.append_move(clonedInstr);
				}
				if (firstInstr)
				{
					jumpTable.SetLocation(ILLabel(block->GetId()), firstInstr);
				}
			}
		}
	}
}