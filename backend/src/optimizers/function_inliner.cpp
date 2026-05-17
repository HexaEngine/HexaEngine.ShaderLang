#include "optimizers/function_inliner.hpp"
#include "il/func_call_graph.hpp"
#include "il/dag_graph.hpp"

namespace HXSL
{
	namespace Backend
	{
		using instr_iterator = BasicBlock::instr_iterator;

		void FunctionInliner::InlineContext::PrepareBlockMapping()
		{
			if (!multiBlockMode)
			{
				return;
			}

			auto* caller = this->caller->GetContext();
			auto* callee = this->callee->GetContext();
			auto& callerCFG = caller->GetCFG();
			auto& calleeCFG = callee->GetCFG();
			auto& calleeBlocks = calleeCFG.GetNodes();

			for (size_t i = 1; i < calleeBlocks.size(); ++i)
			{
				auto& calleeBlock = calleeBlocks[i];
				auto nodeId = callerCFG.AddNode(calleeBlock->GetType(), true);

				blockMap.insert({ ILLabel(calleeBlock->GetId()), ILLabel(nodeId) });
			}

			for (size_t i = 1; i < calleeBlocks.size(); ++i)
			{
				auto& calleeBlock = calleeBlocks[i];
				auto mappedBlockId = blockMap[ILLabel(calleeBlock->GetId())].value;

				for (auto predId : calleeBlock->GetPredecessors())
				{
					auto predIdMapped = blockMap[ILLabel(predId)].value;
					callerCFG.Link(predIdMapped, mappedBlockId);
				}

				auto& callerBlock = callerCFG.GetNode(mappedBlockId);
				if (callerBlock->GetType() == ControlFlowType_Exit)
				{
					callerBlock->SetType(ControlFlowType_Unconditional);
					callerCFG.Link(mappedBlockId, exitNode->GetId());
					// do not eagerly insert jump instruction here, this can cause issues with remapping.
				}
			}
		}

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

		void FunctionInliner::InlineContext::CloneInstruction(Instruction& instr, BasicBlock* block, BasicBlock::instr_iterator insertTarget)
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

				return;
			}
			else if (auto retInstr = dyn_cast<ReturnInstr>(&instr))
			{
				auto src = retInstr->GetReturnValue();
				Instruction* moveInstr = nullptr;
				
				if (auto var = dyn_cast<Variable>(src))
				{
					auto itt = varIdMap.find(var->varId);
					HXSL_ASSERT(itt != varIdMap.end(), "Variable has no mapping, this should never happen while inlining.");
					auto varId = itt->second;

					if (multiBlockMode) // Copy propagation optimization for multi-block, because we insert a phi at the end. 
					{
						returnVars.push_back(varId);
					}
					else
					{
						auto callSiteRetId = AddReturnVar();
						moveInstr = block->InsertInstrO<MoveInstr>(insertTarget, callSiteRetId, varId);
					}
				}
				else if (auto constant = dyn_cast<Constant>(src))
				{
					moveInstr = block->InsertInstrO<MoveInstr>(insertTarget, AddReturnVar(), constant->imm());
				}
				else
				{
					HXSL_ASSERT(false, "Unhandled return value type in inliner.");
				}

				if (multiBlockMode)
				{
					block->AddInstrNO<JumpInstr>(OpCode_Jump, ILLabel(exitNode->GetId()));
				}

				return;
			}

			auto& callerCFG = caller->GetContext()->GetCFG();
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
				else if (auto label = dyn_cast<Label>(op))
				{
					auto it = blockMap.find(label->label);
					HXSL_ASSERT(it != blockMap.end(), "Block has no mapping, this should never happen while inlining.");
					label->label = it->second;
				}
			}

			block->InsertInstr(insertTarget, clonedInstr);
		}

		void FunctionInliner::InlineAtSite(FunctionLayout* callerLayout, FunctionLayout* calleeLayout, CallInstr* site)
		{
			InlineContext ctx = { callerLayout, calleeLayout, site, nullptr, false };

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

			ctx.returnVarId = site->GetResult();
			ctx.exitNode = block;
			bool domTreeAltered = false;
			auto& calleeBlocks = calleeCFG.GetNodes();

			ctx.blockMap.insert({ ILLabel(calleeBlocks[0]->GetId()), ILLabel(block->GetId()) });
			if (calleeBlocks.size() > 1) // split block.
			{
				auto newNodeId = callerCFG.SplitNode(block->GetId(), insertTarget);
				callerCFG.Unlink(block->GetId(), newNodeId);
				ctx.exitNode = callerCFG.GetNode(newNodeId).get();
				ctx.multiBlockMode = true;
				ctx.returnVarId.IncrementVersion();
				domTreeAltered = true;
			}

			ctx.PrepareBlockMapping();

			for (auto& calleeBlock : calleeBlocks)
			{
				auto targetBlockId = ctx.blockMap[ILLabel(calleeBlock->GetId())];
				auto targetBlock = callerCFG.GetNode(targetBlockId.value).get();
				auto insertTarget = targetBlock->end();
				for (auto& instr : *calleeBlock)
				{
					ctx.CloneInstruction(instr, targetBlock, insertTarget);
				}
			}
			if (ctx.multiBlockMode && ctx.returnVars.size() > 0)
			{
				auto exitNode = ctx.exitNode;
				auto& returnVars = ctx.returnVars;
				auto returnVarCount = returnVars.size();
				auto phi = cast<PhiInstr>(exitNode->ReplaceInstrO<PhiInstr>(site, site->GetResult(), returnVarCount));
				for (size_t i = 0; i < returnVarCount; ++i)
				{
					phi->GetOperands()[i] = exitNode->GetParent()->Alloc<Variable>(returnVars[i]);
				}
				caller->metadata.phiNodes.push_back(phi);
			}
			else
			{
				HXSL_ASSERT(ctx.returnVars.size() <= 1, "Single block merge has multiple return vars, this should never happen.");
				ctx.exitNode->RemoveInstr(site);
			}

			if (domTreeAltered)
			{
				callerCFG.RebuildDomTree();
			}
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

			totalCost += (1 - std::exp(-static_cast<float>(instrCount) * heuristics.InstrCostExpMul)) * heuristics.InstrCostMul;
			totalCost = std::clamp(totalCost, heuristics.MinInlineCost, heuristics.MaxInlineCost);
			return totalCost;
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

			bool inlineExtern = options.Get<InlinerInlineExtern>();
			dense_set<FunctionLayout*> dirtyFunctions;
			static constexpr float MaxCost = 2.0f;
			for (auto callerScc : sccOrder)
			{
				for (size_t callerNode : sccs[callerScc])
				{
					auto* callerLayout = nodes[callerNode]->GetFunction();
					auto* caller = callerLayout->GetContext();
					auto& metadata = caller->metadata;
					if (caller->empty() || (caller->IsExtern() && !inlineExtern)) continue;

					for (auto& call : metadata.functions)
					{
						auto* calleeLayout = call->func;
						size_t calleeNode = callGraph.GetIndex(calleeLayout);
						size_t calleeScc = nodes[calleeNode]->GetSCCIndex();

						if (callerScc == calleeScc || (calleeLayout->IsExtern() && !inlineExtern))
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