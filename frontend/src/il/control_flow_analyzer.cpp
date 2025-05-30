#include "control_flow_analyzer.hpp"

namespace HXSL
{
	namespace Backend
	{
		void ControlFlowAnalyzer::DetectUnreachableCode(ILContext* function)
		{
			auto& cfg = function->GetCFG();
			auto& metadata = function->GetMetadata();

			bool changed = false;
			auto& nodes = cfg.GetNodes();
			for (size_t i = 1; i < nodes.size(); i++)
			{
				auto& node = nodes[i];
				if (node.IsPredecessorsEmpty())
				{
					TextSpan span;
					bool first = true;
					for (auto& instr : node)
					{
						auto loc = instr.GetLocation();
						if (!loc) continue;
						if (first)
						{
							span = *loc;
							first = false;
						}
						else
						{
							span.merge(*loc);
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
			auto& functions = module->GetAllFunctions();

			for (auto& func : functions)
			{
				DetectUnreachableCode(func->GetContext());
			}
		}
	}
}