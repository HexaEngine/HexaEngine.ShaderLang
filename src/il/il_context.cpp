#include "il_context.hpp"
#include "optimizers/constant_folder.hpp"
#include "optimizers/algebraic_simplifier.hpp"
#include "optimizers/function_inliner.hpp"
#include "il/control_flow_graph.hpp"
#include "il/control_flow_analyzer.hpp"
#include "ssa/ssa_builder.hpp"

namespace HXSL
{
	void ILContext::Build()
	{
		builder.Build(overload);
	}

	void ILContext::Fold()
	{
	}

	void ILContext::BuildCFG()
	{
		ControlFlowGraph cfg(builder);
		cfg.Build();

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
					auto map = builder.FindMappingForInstruction(instr + j);
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

		SSABuilder ssaBuilder = SSABuilder(builder, cfg);
		ssaBuilder.Build();

		while (true)
		{
			cfg.Print(builder.GetMetadata());
			ConstantFolder folder(cfg, builder);
			folder.Traverse();

			AlgebraicSimplifier algSimp(builder, cfg);
			algSimp.Traverse();
			if (algSimp.HasChanged())
			{
				continue;
			}

			break;
		}

		cfg.Print(builder.GetMetadata());
		return;
	}

	void ILContext::UpdateState()
	{
		auto& container = builder.GetContainer();
		auto& metadata = builder.GetMetadata();
		canInline = builder.GetContainer().size() < 10;
		for (auto& refFunc : metadata.functions)
		{
			if (refFunc.func == overload)
			{
				canInline = false;
				break;
			}
		}
	}

	void ILContext::TryInline(ILContext& ctx, uint64_t funcSlot)
	{
		if (!ctx.canInline) return;
		FunctionInliner inliner = FunctionInliner(builder);
		inliner.Inline(ctx, funcSlot);
	}
}