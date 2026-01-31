#include "optimizers/function_inliner.hpp"
#include "il/func_call_graph.hpp"
#include "il/dag_graph.hpp"

namespace HXSL
{
	namespace Backend
	{
		ILVarId FunctionInliner::InlineContext::RemapVarId(const ILVarId& varId)
		{
			auto* caller = this->caller->GetContext();
			auto* callee = this->callee->GetContext();
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

		/// <summary>
		/// Configuration struct that defines cost heuristics for function inlining decisions. Each member represents a cost metric used to evaluate whether inlining a function is beneficial.
		/// Higher = more expensive, Lower = cheaper
		/// </summary>
		struct InlinerCostHeuristics
		{
			float BaseCost = -10;
			float InstrCostExpMul = 0.015f;
			float InstrCostMul = 25.0f;
			float ControlFlowCost = 10;
			float ReturnCost = -2;
			float LoadParamCost = -3;
			float MemoryOpCost = 1.0f;
			float ArithmeticOpCost = 0.1f;

			float ConstantCost = -2;
			float MaxInlineCost = 20;
			float MinInlineCost = -20;
		};

		float FunctionInliner::ComputeInlineCost(FunctionLayout* funcLayout)
		{
			auto* function = funcLayout->GetContext();
			auto& cfg = function->cfg;
			auto& metadata = function->metadata;

			InlinerCostHeuristics heuristics;

			float totalCost = heuristics.BaseCost;
			size_t instrCount = 0;
			for (auto& block : cfg.GetNodes())
			{
				instrCount += block->GetInstructions().size();
		
				if (block->NumPredecessors() > 1 || block->NumSuccessors() > 1)
				{
					totalCost += heuristics.ControlFlowCost;
				}
				for (auto& instr : *block)
				{
					if (isa<ReturnInstr>(&instr))
					{
						totalCost += heuristics.ReturnCost;
					}
					else if (isa<LoadParamInstr>(&instr))
					{
						totalCost += heuristics.LoadParamCost;
					}
					else if (isa<LoadInstr>(&instr) || isa<StoreInstr>(&instr) || isa<StackAllocInstr>(&instr))
					{
						totalCost += heuristics.MemoryOpCost;
					}
					else if (isa<BinaryInstr>(&instr) || isa<UnaryInstr>(&instr))
					{
						totalCost += heuristics.ArithmeticOpCost;
					}
					for (auto& op : instr.GetOperands())
					{
						if (auto constant = dyn_cast<Constant>(op))
						{
							totalCost += heuristics.ConstantCost;
						}
					}
				}
			}

			totalCost += (1 - std::exp(-instrCount * heuristics.InstrCostExpMul)) * heuristics.InstrCostMul;
			totalCost = std::clamp(totalCost, heuristics.MinInlineCost, heuristics.MaxInlineCost);
			return totalCost;
		}

		void FunctionInliner::InlineAtSite(FunctionLayout* callerLayout, FunctionLayout* calleeLayout, CallInstr* site)
		{
			InlineContext ctx = {callerLayout, calleeLayout, site};

			auto* caller = callerLayout->GetContext();
			auto* callee = calleeLayout->GetContext();
			auto& callerCFG = caller->cfg;
			auto& calleeCFG = callee->cfg;
			auto& callerMetadata = caller->metadata;
			auto& calleeMetadata = callee->metadata;

			auto paramCount = calleeLayout->GetParameters().size();
			ctx.params.resize(paramCount);

			auto prev = site->GetPrev();
			auto block = site->GetParent();
			Instruction* lastArg = site;
	

			auto insertTarget = BasicBlock::instr_iterator(site);

			if (paramCount > 0)
			{
				size_t collectedParams = 0;
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
					auto& info = ctx.params[idx];
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
					if (++collectedParams == paramCount)
					{
						break;
					}
				}
			}

			for (auto& calleeBlock : calleeCFG.GetNodes())
			{
				// TODO: Handle multiple blocks / control flow
				for (auto& instr : *calleeBlock)
				{
					if (auto loadParam = dyn_cast<LoadParamInstr>(&instr))
					{
						auto idx = loadParam->GetParamIdx();
						auto& info = ctx.params[idx];
						auto dst = loadParam->GetResult();
						if (info.type == ParamInfoType::Imm)
						{
							auto newVarId = ctx.RemapVarId(dst);
							block->InsertInstrO<MoveInstr>(insertTarget, newVarId, info.imm);
						}
						else if (info.type == ParamInfoType::VarId)
						{
							ctx.varIdMap[dst] = info.varId;
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
							auto itt = ctx.varIdMap.find(var->varId);
							HXSL_ASSERT(itt != ctx.varIdMap.end(), "Variable has no mapping, this should never happen while inlining.");
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
						ILVarId newVarId = ctx.RemapVarId(varId);
						resInstr->SetResult(newVarId);
					}

					for (auto& op : clonedInstr->GetOperands())
					{
						if (auto var = dyn_cast<Variable>(op))
						{
							auto it = ctx.varIdMap.find(var->varId);
							HXSL_ASSERT(it != ctx.varIdMap.end(), "Variable has no mapping, this should never happen while inlining.");
							var->varId = it->second;
						}
					}

					block->InsertInstr(insertTarget, clonedInstr);
				}
			}

			block->RemoveInstr(site);
		}

		dense_set<FunctionLayout*> FunctionInliner::Inline(const Span<FunctionLayout*> functions)
		{
			FuncCallGraph callGraph = FuncCallGraph();

			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				if (function->empty()) continue;
				auto* node = callGraph.AddFunction(functionLayout);
				node->SetInlineCost(ComputeInlineCost(functionLayout));
			}

			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				if (function->empty()) continue;
				auto& metadata = function->metadata;
				for (auto& call : metadata.functions)
				{
					callGraph.AddCall(functionLayout, call->func);
				}
			}

			callGraph.UpdateSCCs();

			auto& nodes = callGraph.GetNodes();
			auto& sccs = callGraph.GetSCCs();

			DAGGraph<size_t> sccGraph = DAGGraph<size_t>();
			for (size_t scc = 0; scc < sccs.size(); ++scc)
			{
				sccGraph.AddNode(scc);
			}

			for (size_t u = 0; u < nodes.size(); ++u)
			{
				auto& node = nodes[u];
				size_t su = node->GetSCCIndex();
				for (size_t v : nodes[u]->GetDependencies())
				{
					size_t sv = nodes[v]->GetSCCIndex();
					if (su != sv)
					{
						sccGraph.AddEdge(su, sv);
					}
				}
			}

			std::vector<size_t> sccOrder = sccGraph.TopologicalSort(true); // true == bottom-up order

			dense_set<FunctionLayout*> dirtyFunctions;
			static constexpr float MaxCost = 2.0f;
			for (auto callerScc : sccOrder)
			{
				for (size_t callerNode : sccs[callerScc])
				{
					auto* callerLayout = nodes[callerNode]->GetFunction();
					auto* caller = callerLayout->GetContext();
					auto& metadata = caller->metadata;
					if (caller->empty()) continue;

					for (auto& call : metadata.functions)
					{
						auto* calleeLayout = call->func;
						size_t calleeNode = callGraph.GetIndex(calleeLayout);
						size_t calleeScc = nodes[calleeNode]->GetSCCIndex();

						if (callerScc == calleeScc)
						{
							continue;
						}

						auto* callee = calleeLayout->GetContext();
						float inlineCost = callGraph.GetNode(calleeLayout)->GetInlineCost();
						if (inlineCost > MaxCost)
						{
							continue;
						}

						InlineAtAllSites(callerLayout, calleeLayout, call->callSites);
						caller->metadata.RemoveFunc(call);

#if HXSL_DEBUG
						std::cout << "Inliner:" << std::endl;
						caller->cfg.Print();
#endif
						dirtyFunctions.insert(callerLayout);
					}
				}
			}

			return dirtyFunctions;
		}
	}	
}