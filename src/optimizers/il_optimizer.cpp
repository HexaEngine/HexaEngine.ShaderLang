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

			SSABuilder ssaBuilder = SSABuilder(metadata, cfg);
			ssaBuilder.Build();

			Optimize(function.get());

			SSAReducer reducer = SSAReducer(metadata, cfg);
			reducer.Reduce();
#if HXSL_DEBUG
			cfg.Print();
#endif
		}
	}

	static std::vector<std::unique_ptr<ILOptimizerPass>> MakeScope(ILFunction* function)
	{
		auto& cfg = function->cfg;
		auto& metadata = function->metadata;

		std::vector<std::unique_ptr<ILOptimizerPass>> passes;
		passes.push_back(std::make_unique<ConstantFolder>(metadata, cfg));
		passes.push_back(std::make_unique<AlgebraicSimplifier>(metadata, cfg));
		passes.push_back(std::make_unique<CommonSubExpression>(metadata, cfg));
		passes.push_back(std::make_unique<DeadCodeEliminator>(metadata, cfg));
		return std::move(passes);
	}

	void ILOptimizer::Optimize(ILFunction* function)
	{
#if HXSL_DEBUG
		auto& cfg = function->cfg;
		cfg.Print();
#endif
		auto passes = MakeScope(function);

		for (size_t i = 0; i < 10; i++)
		{
			PROFILE_SCOPE("Optimizer");
			bool changed = false;
			for (auto& pass : passes)
			{
				auto result = pass->Run();
				if (result == OptimizerPassResult_Rerun)
				{
#if HXSL_DEBUG
					cfg.Print();
#endif
					break;
				}
				else if (result == OptimizerPassResult_Changed)
				{
#if HXSL_DEBUG
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