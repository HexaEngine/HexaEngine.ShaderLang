#ifndef SSA_BUILDER_HPP
#define SSA_BUILDER_HPP

#include "ssa_instruction.hpp"
#include "il/control_flow_graph.hpp"
#include "il/lt_dominator_tree.hpp"

namespace HXSL
{
	struct SSACFGContext
	{
		std::vector<uint64_t> variables;
	};

	class SSABuilder : CFGVisitor<SSACFGContext>
	{
		ILBuilder& builder;
		ControlFlowGraph& cfg;

		std::vector<std::vector<size_t>>& domTreeChildren;
		std::vector<std::unordered_set<size_t>>& domFront;

		std::unordered_map<uint64_t, std::stack<uint64_t>> versionStacks;
		std::unordered_map<uint64_t, uint32_t> versionCounters;
		std::unordered_map<uint64_t, std::unordered_set<size_t>> defSites;

		ILRegister currentTemp = 0;
		std::unordered_map<ILRegister, ILRegister> r2r;

		std::vector<size_t> discardList;

		uint64_t TopVersion(uint64_t varId)
		{
			if (versionStacks[varId].empty())
				return varId;
			return versionStacks[varId].top();
		}

		uint64_t MakeNewVersion(uint64_t varId, bool push = true)
		{
			uint32_t version = ++versionCounters[varId];
			uint64_t newVersion = MakeVersion(varId, version);
			if (push)
			{
				versionStacks[varId].push(newVersion);
			}
			return newVersion;
		}

		void PushVersion(uint64_t varId, uint64_t newVersion)
		{
			versionStacks[varId].push(newVersion);
		}

		void PopVersion(uint64_t varId)
		{
			versionStacks[varId].pop();
		}

		void DiscardMarkedInstructs(CFGNode& node)
		{
			auto& container = node.instructions;
			size_t toDiscard = discardList.size();
			size_t discardIndex = 0;
			size_t writeIndex = 0;
			for (size_t i = 0; i < container.size(); i++)
			{
				auto& instr = container[i];
				if (discardIndex < toDiscard && discardList[discardIndex] == i)
				{
					discardIndex++;
					continue;
				}

				container[writeIndex] = std::move(instr);
				writeIndex++;
			}
			container.resize(writeIndex);

			discardList.clear();
		}

		void DiscardInstr(size_t index)
		{
			auto it = std::lower_bound(discardList.begin(), discardList.end(), index);
			if (it == discardList.end() || *it != index)
			{
				discardList.insert(it, index);
			}
		}

		void MapTempRegister(ILInstruction& instr);

		void InsertPhiMeta(CFGNode& node, uint64_t varId, size_t& phiIdOut);

		void Visit(size_t index, CFGNode& node, SSACFGContext& context) override;

		void VisitClose(size_t index, CFGNode& node, SSACFGContext& context) override;

	public:
		SSABuilder(ILBuilder& builder, ControlFlowGraph& cfg) : builder(builder), CFGVisitor(cfg), cfg(cfg), domTreeChildren(cfg.domTreeChildren), domFront(cfg.domFront)
		{
		}

		void Build();
	};
}

#endif