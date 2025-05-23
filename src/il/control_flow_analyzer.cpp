#include "control_flow_analyzer.hpp"

namespace HXSL
{
	void ControlFlowAnalyzer::DetectUnreachableCode(ILFunction* function)
	{
		auto& cfg = function->cfg;
		auto& metadata = function->metadata;

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
	}

	void ControlFlowAnalyzer::Analyze()
	{
		auto& functions = compilation->GetILFunctionsMut();

		for (auto& func : functions)
		{
			DetectUnreachableCode(func.get());
		}
	}
}