#include "il_optimizer.hpp"

#include "il/ssa/ssa_builder.hpp"

#include "optimizers/constant_folder.hpp"
#include "optimizers/algebraic_simplifier.hpp"
#include "optimizers/strength_reduction.hpp"
#include "optimizers/common_sub_expression.hpp"
#include "optimizers/function_inliner.hpp"

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
		}
	}

	void ILOptimizer::Optimize(ILFunction* function)
	{
		auto& cfg = function->cfg;
		auto& metadata = function->metadata;

		for (size_t i = 0; i < 10; i++)
		{
			cfg.Print();
			ConstantFolder folder(cfg, metadata);
			folder.Traverse();

			AlgebraicSimplifier algSimp(cfg, metadata);
			algSimp.Traverse();
			if (algSimp.HasChanged())
			{
				continue;
			}

			StrengthReduction sr(cfg, metadata);
			sr.Traverse();

			CommonSubExpression cse(cfg, metadata);
			cse.Traverse();
		}

		cfg.Print();
	}
}