#ifndef SSA_BUILDER_HPP
#define SSA_BUILDER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	struct SSACFGContext
	{
		std::vector<uint64_t> variables;
	};

	class SSABuilder : CFGVisitor<SSACFGContext>
	{
		ILMetadata& metadata;
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

		void MapTempRegister(ILInstruction& instr);

		void InsertPhiMeta(CFGNode& node, uint64_t varId, size_t& phiIdOut);

		void Visit(size_t index, CFGNode& node, SSACFGContext& context) override;

		void VisitClose(size_t index, CFGNode& node, SSACFGContext& context) override;

	public:
		SSABuilder(ILMetadata& metadata, ControlFlowGraph& cfg) : metadata(metadata), CFGVisitor(cfg), cfg(cfg), domTreeChildren(cfg.domTreeChildren), domFront(cfg.domFront)
		{
		}

		void Build();
	};
}

#endif