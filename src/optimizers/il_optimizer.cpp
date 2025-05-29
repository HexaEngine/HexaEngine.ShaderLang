#include "il_optimizer.hpp"

#include "il/ssa/ssa_builder.hpp"
#include "il/ssa/ssa_reducer.hpp"
#include "il_optimizer_pass.hpp"

#include "constant_folder.hpp"
#include "algebraic_simplifier.hpp"
#include "strength_reduction.hpp"
#include "common_sub_expression.hpp"
#include "dead_code_eliminator.hpp"
#include "function_inliner.hpp"

#include "utils/scoped_timer.hpp"

namespace HXSL
{
	void ILOptimizer::Optimize()
	{
		auto& functions = compilation->GetILFunctionsMut();

		for (auto& function : functions)
		{
			auto& cfg = function->cfg;
			auto& metadata = function->metadata;

			if (function->empty()) continue;

			SSABuilder ssaBuilder = SSABuilder(function.get());
			ssaBuilder.Build();

#if HXSL_DEBUG
			std::cout << "Converted IL to SSA:" << std::endl;
			cfg.Print();
#endif

			Optimize(function.get());

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
		passes.push_back(std::make_unique<CommonSubExpression>(function));
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