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
#include "il/il_code_blob.hpp"



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
			}

			FunctionInliner inliner = FunctionInliner();
			for (size_t i = 0; i < 10; ++i)
			{	
				auto inlined = inliner.Inline(functions);
				if (inlined.empty())
				{
					break;
				}
				for (auto& funcLayout : inlined)
				{
					Optimize(funcLayout->GetContext());
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
				auto& alloc = module->GetAllocator();
				ILCodeBlob* ilBlob = alloc.Alloc<ILCodeBlob>();
		
				ilBlob->FromContext(function);

#if HXSL_DEBUG
				std::cout << "Final IL:" << std::endl;
				std::cout << functionLayout->ToString() << std::endl;
				ilBlob->Print();
#endif
				functionLayout->SetCodeBlob(ilBlob);
			}
		}

		static std::vector<uptr<ILOptimizerPass>> MakeScope(ILContext* function)
		{
			std::vector<uptr<ILOptimizerPass>> passes;
			passes.push_back(make_uptr<ConstantFolder>(function));
			passes.push_back(make_uptr<AlgebraicSimplifier>(function));
			passes.push_back(make_uptr<ReassociationPass>(function));
			passes.push_back(make_uptr<GlobalValueNumbering>(function));
			passes.push_back(make_uptr<DeadCodeEliminator>(function));
			//passes.push_back(make_uptr<StrengthReduction>(function));
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
						changed = true;
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