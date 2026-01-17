#include "optimizers/function_inliner.hpp"
#include "il/func_call_graph.hpp"
#include "il/dag_graph.hpp"

namespace HXSL
{
	namespace Backend
	{
		ILVarId FunctionInliner::RemapVarId(const ILVarId& varId)
		{
			auto* caller = callerLayout->GetContext();
			auto* callee = calleeLayout->GetContext();
			auto& callerMetadata = caller->metadata;
			auto& calleeMetadata = callee->metadata;

			ILVarId newVarId;
			auto strippedVarId = varId.StripVersion();
			auto it = baseVarMap.find(strippedVarId);

			if (it != baseVarMap.end())
			{
				newVarId = it->second;
			}
			else
			{
				auto& var = calleeMetadata.GetVar(varId);
				auto& newVar = callerMetadata.CloneVar(varId, var);
				baseVarMap[strippedVarId] = newVar.id;
				newVarId = newVar.id;
			}

			newVarId = newVarId.WithVersion(varId.version());
			varIdMap[varId] = newVarId;
			return newVarId;
		}

		void FunctionInliner::InlineAt(CallInstr* site)
		{
			params.clear();
			baseVarMap.clear();
			varIdMap.clear();

			auto* caller = callerLayout->GetContext();
			auto* callee = calleeLayout->GetContext();
			auto& callerCFG = caller->cfg;
			auto& calleeCFG = callee->cfg;
			auto& callerMetadata = caller->metadata;
			auto& calleeMetadata = callee->metadata;

			auto paramCount = calleeLayout->GetParameters().size();
			params.resize(paramCount);

			auto prev = site->GetPrev();
			auto block = site->GetParent();
			Instruction* lastArg = site;
	

			auto insertTarget = BasicBlock::instr_iterator(site);

			while (prev)
			{
				auto arg = dyn_cast<StoreParamInstr>(prev);
				if (!arg)
				{
					prev = prev->GetPrev();
					continue;
				}

				auto src = arg->GetSource();
				auto idx = arg->GetParamIdx();
				auto& info = params[idx];
				if (auto constant = dyn_cast<Constant>(src))
				{
					auto imm = constant->imm();
					info.type = ParamInfoType::Imm;
					info.imm = imm;
				}
				else if (auto var = dyn_cast<Variable>(src))
				{
					info.type = ParamInfoType::VarId;
					info.varId = var->varId;
				}
				else
				{
					HXSL_ASSERT(false, "Unhandled param type in function inliner.")
				}

				auto next = prev->GetPrev();
				block->RemoveInstr(prev);
				prev = next;
			}


			for (auto& calleeBlock : calleeCFG.GetNodes())
			{
				for (auto& instr : *calleeBlock)
				{
					if (auto loadParam = dyn_cast<LoadParamInstr>(&instr))
					{
						auto idx = loadParam->GetParamIdx();
						auto& info = params[idx];
						auto dst = loadParam->GetResult();
						if (info.type == ParamInfoType::Imm)
						{
							auto newVarId = RemapVarId(dst);
							block->InsertInstrO<MoveInstr>(insertTarget, newVarId, info.imm);
						}
						else if (info.type == ParamInfoType::VarId)
						{
							varIdMap[dst] = info.varId;
						}
						else 
						{
							HXSL_ASSERT(false, "Unknown parameter info type in inliner.");
						}

						continue;
					}
					else if (auto retInstr = dyn_cast<ReturnInstr>(&instr))
					{
						auto src = retInstr->GetReturnValue();
						if (auto var = dyn_cast<Variable>(src))
						{
							auto itt = varIdMap.find(var->varId);
							HXSL_ASSERT(itt != varIdMap.end(), "Variable has no mapping, this should never happen while inlining.");
							auto varId = itt->second;

							block->InsertInstrO<MoveInstr>(insertTarget, site->GetResult(), varId);
						}
						else if (auto constant = dyn_cast<Constant>(src))
						{
							block->InsertInstrO<MoveInstr>(insertTarget, site->GetResult(), constant->imm());
						}
						else
						{
							HXSL_ASSERT(false, "Unhandled return value type in inliner.");
						}

						continue;
					}

					auto clonedInstr = instr.Clone(callerCFG.allocator);
					if (auto resInstr = dyn_cast<ResultInstr>(clonedInstr))
					{
						auto varId = resInstr->GetResult();
						ILVarId newVarId = RemapVarId(varId);
						resInstr->SetResult(newVarId);
					}

					for (auto& op : clonedInstr->GetOperands())
					{
						if (auto var = dyn_cast<Variable>(op))
						{
							auto it = varIdMap.find(var->varId);
							HXSL_ASSERT(it != varIdMap.end(), "Variable has no mapping, this should never happen while inlining.");
							var->varId = it->second;
						}
					}

					block->InsertInstr(insertTarget, clonedInstr);
				}
			}

			block->RemoveInstr(site);
		}
	}	
}