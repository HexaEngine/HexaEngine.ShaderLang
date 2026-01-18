#include "optimizers/il_optimizer.hpp"

#include "ssa/ssa_builder.hpp"
#include "ssa/ssa_reducer.hpp"
#include "optimizers/il_optimizer_pass.hpp"

#include "optimizers/constant_folder.hpp"
#include "optimizers/algebraic_simplifier.hpp"
#include "optimizers/reassociation_pass.hpp"
#include "optimizers/strength_reduction.hpp"
#include "optimizers/global_value_numbering.hpp"
#include "optimizers/dead_code_eliminator.hpp"
#include "optimizers/function_inliner.hpp"
#include "optimizers/loop_unroller.hpp"
#include "il/func_call_graph.hpp"
#include "il/loop_tree.hpp"
#include "il/dag_graph.hpp"
#include "il/rpo_merger.hpp"


#include "utils/scoped_timer.hpp"

namespace HXSL
{
	namespace Backend
	{
		void ILOptimizer::OptimizeFunctions(const Span<FunctionLayout*>& functions)
		{
			for (auto& functionLayout : functions)
			{
				Optimize(functionLayout->GetContext());
			}
		}

		struct ILModule
		{
			ILContext* context;
			ILContainer& container;
			JumpTable& jumpTable;

			void Print()
			{
				std::cout << "{" << std::endl;
				for (auto& instr : container)
				{
					size_t offset = 0;
					while (true)
					{
						auto it = std::find(jumpTable.locations.begin() + offset, jumpTable.locations.end(), &instr);
						if (it == jumpTable.locations.end())
						{
							break;
						}
						auto index = std::distance(jumpTable.locations.begin(), it);
						std::cout << "loc_" << index << ":" << std::endl;
						offset = index + 1;
					}
					std::cout << "    " << ToString(instr, context->GetMetadata()) << std::endl;
				}
				std::cout << "}" << std::endl;
			}
		};

		void ILOptimizer::Optimize()
		{
			auto& functions = module->GetAllFunctions();
			FuncCallGraph callGraph = FuncCallGraph();
			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				auto& cfg = function->cfg;
				auto& metadata = function->metadata;

				if (function->empty()) continue;

				SSABuilder ssaBuilder = SSABuilder(function);
				ssaBuilder.Build();

#if HXSL_DEBUG
				std::cout << "Converted IL to SSA:" << std::endl;
				cfg.Print();
#endif

				Optimize(function);

				if (!function->empty())
				{
					callGraph.AddFunction(functionLayout);
				}
			}

			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				auto& cfg = function->cfg;
				auto& metadata = function->metadata;
				if (function->empty()) continue;
				for (auto& call : metadata.functions)
				{
					callGraph.AddCall(functionLayout, call->func);
				}
			}

			callGraph.UpdateSCCs();
		
			auto& nodes = callGraph.GetNodes();
			auto& sccs = callGraph.GetSCCs();

			std::vector<size_t> nodeToScc(nodes.size());
			for (size_t scc = 0; scc < sccs.size(); ++scc)
			{
				for (size_t n : sccs[scc])
				{
					nodeToScc[n] = scc;
				}
			}

			DAGGraph<size_t> sccGraph = DAGGraph<size_t>();
			for (size_t scc = 0; scc < sccs.size(); ++scc)
			{
				sccGraph.AddNode(scc);
			}

			for (size_t u = 0; u < nodes.size(); ++u)
			{
				size_t su = nodeToScc[u];
				for (size_t v : nodes[u]->dependencies)
				{
					size_t sv = nodeToScc[v];
					if (su != sv)
					{
						sccGraph.AddEdge(su, sv);
					}
				}
			}

			std::vector<size_t> sccOrder = sccGraph.TopologicalSort();

			for (auto callerScc : sccOrder)
			{
				for (size_t callerNode : sccs[callerScc])
				{
					auto* callerLayout = nodes[callerNode]->function;
					auto* caller = callerLayout->GetContext();
					auto& metadata = caller->metadata;
					if (caller->empty()) continue;

					for (auto& call : metadata.functions)
					{
						auto* calleeLayout = call->func;
						size_t calleeNode = callGraph.GetIndex(calleeLayout);
						size_t calleeScc = nodeToScc[calleeNode];
					
						if (callerScc == calleeScc)
						{
							continue;
						}

						auto* callee = calleeLayout->GetContext();
						auto instructionCount = callee->cfg.CountInstructions();
						if (instructionCount > 20)
						{
							continue;
						}

						FunctionInliner inliner = FunctionInliner(callerLayout, calleeLayout);
						inliner.InlineAll(call->callSites);

						std::cout << "Inliner:" << std::endl;
						caller->cfg.Print();
						Optimize(caller);
					}
				}
			}

			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				auto& cfg = function->cfg;
				auto& metadata = function->metadata;
				if (function->empty()) continue;

				auto& loopTree = function->loopTree;
				loopTree.Build();
#if HXSL_DEBUG
				std::cout << "Loop Tree:" << std::endl;
				loopTree.Print();
#endif
				LoopUnroller unroller = LoopUnroller(function);
				auto result = unroller.Run();
				if (result == OptimizerPassResult_Changed)
				{
#if HXSL_DEBUG
					std::cout << "After Loop Unrolling:" << std::endl;
					cfg.Print();
#endif
					Optimize(function);
				}
			}

			for (auto& functionLayout : functions)
			{
				auto function = functionLayout->GetContext();
				auto& cfg = function->cfg;
				auto& metadata = function->metadata;
				if (function->empty()) continue;

				SSAReducer reducer = SSAReducer(metadata, cfg);
				reducer.Reduce();

#if HXSL_DEBUG
				std::cout << "Lowered SSA to IL:" << std::endl;
				cfg.Print();
#endif
				RPOMerger rpoMerger = RPOMerger(cfg);
				ILContainer newContainer = ILContainer(function->allocator);
				JumpTable newJumpTable = JumpTable();
				newJumpTable.Resize(cfg.size());
				rpoMerger.Merge(newContainer, newJumpTable);

				ILModule module = ILModule{ function, newContainer, newJumpTable };
				std::cout << "Final IL:" << std::endl;
				module.Print();
				
			}
		}

		static std::vector<uptr<ILOptimizerPass>> MakeScope(ILContext* function)
		{
			std::vector<uptr<ILOptimizerPass>> passes;
			passes.push_back(make_uptr<ConstantFolder>(function));
			passes.push_back(make_uptr<AlgebraicSimplifier>(function));
			//passes.push_back(make_uptr<StrengthReduction>(function));
			passes.push_back(make_uptr<ReassociationPass>(function));
			passes.push_back(make_uptr<GlobalValueNumbering>(function));
			passes.push_back(make_uptr<DeadCodeEliminator>(function));
			return passes;
		}

		void ILOptimizer::Optimize(ILContext* function)
		{
#if HXSL_DEBUG
			auto& cfg = function->cfg;
#endif
			auto passes = MakeScope(function);

			//PROFILE_SCOPE("Optimizer Main");
			for (size_t i = 0; i < 10; i++)
			{
				//PROFILE_SCOPE("Optimizer Pass");
				bool changed = false;
				for (auto& pass : passes)
				{
					//PROFILE_SCOPE("Optimizer Sub Pass");
					auto result = pass->Run();
					if (result == OptimizerPassResult_Rerun)
					{
#if HXSL_DEBUG
						std::cout << "Pass: " << pass->GetName() << std::endl;
						cfg.Print();
#endif
						break;
					}
					else if (result == OptimizerPassResult_Changed)
					{
#if HXSL_DEBUG
						std::cout << "Pass: " << pass->GetName() << std::endl;
						cfg.Print();
#endif
						changed = true;
					}
				}

				if (!changed)
				{
					break;
				}
			}
		}
	}
}