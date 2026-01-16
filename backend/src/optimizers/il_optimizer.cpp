#include "optimizers/il_optimizer.hpp"

#include "ssa/ssa_builder.hpp"
#include "ssa/ssa_reducer.hpp"
#include "optimizers/il_optimizer_pass.hpp"

#include "optimizers/constant_folder.hpp"
#include "optimizers/algebraic_simplifier.hpp"
#include "optimizers/strength_reduction.hpp"
#include "optimizers/common_sub_expression.hpp"
#include "optimizers/dead_code_eliminator.hpp"
#include "optimizers/function_inliner.hpp"
#include "il/func_call_graph.hpp"
#include "il/dag_graph.hpp"

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

				SSAReducer reducer = SSAReducer(metadata, cfg);
				reducer.Reduce();

#if HXSL_DEBUG
				std::cout << "Lowered SSA to IL:" << std::endl;
				cfg.Print();
#endif
			}
		}

		static std::vector<std::unique_ptr<ILOptimizerPass>> MakeScope(ILContext* function)
		{
			std::vector<std::unique_ptr<ILOptimizerPass>> passes;
			passes.push_back(std::make_unique<ConstantFolder>(function));
			passes.push_back(std::make_unique<AlgebraicSimplifier>(function));
			passes.push_back(std::make_unique<CommonSubExpression>(function)); // CSE is technically a GVN but that's just semantics...
			passes.push_back(std::make_unique<DeadCodeEliminator>(function));
			return std::move(passes);
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