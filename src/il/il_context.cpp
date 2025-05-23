#include "il_context.hpp"
#include "il_builder.hpp"
#include "control_flow_graph.hpp"
#include "control_flow_analyzer.hpp"

#include "optimizers/constant_folder.hpp"
#include "optimizers/algebraic_simplifier.hpp"
#include "optimizers/strength_reduction.hpp"
#include "optimizers/common_sub_expression.hpp"
#include "optimizers/function_inliner.hpp"

#include "ssa/ssa_builder.hpp"

namespace HXSL
{
	void ILContext::Build()
	{
		ILContainer container = {};
		JumpTable jumpTable = {};

		ILBuilder builder = ILBuilder(container, metadata, jumpTable);
		builder.Build(overload);

		cfg.Build(container, jumpTable);
	}

	void ILContext::BuildCFG()
	{
		bool changed = false;
		auto& nodes = cfg.GetNodes();
		for (size_t i = 1; i < nodes.size(); i++)
		{
			auto& node = nodes[i];
			if (node.predecessors.empty())
			{
				auto instr = node.startInstr;
				TextSpan span;
				for (size_t j = 0; j < node.instructions.size(); j++)
				{
					auto map = metadata.FindMappingForInstruction(instr + j);
					if (map)
					{
						if (j != 0)
						{
							span.merge(map->span);
						}
						else
						{
							span = map->span;
						}
					}
				}

				Log(UNREACHABLE_CODE, span);

				cfg.RemoveNode(i);
				changed = true;
				i--;
			}
		}

		if (changed)
		{
			cfg.RebuildDomTree();
		}

		SSABuilder ssaBuilder = SSABuilder(metadata, cfg);
		ssaBuilder.Build();

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
		return;
	}

	void ILContext::UpdateState()
	{
	}

	void ILContext::TryInline(ILContext& ctx, uint64_t funcSlot)
	{
	}
}