#ifndef SSA_BUILDER_HPP
#define SSA_BUILDER_HPP

#include "pch/il.hpp"
#include "il/memory_ssa_graph.hpp"

namespace HXSL
{
	struct SSACFGContext
	{
		std::vector<ILVarId> variables;
	};

	class SSABuilder : CFGVisitor<SSACFGContext>
	{
		ILContext* context;
		ILMetadata& metadata;

		std::vector<std::vector<size_t>>& domTreeChildren;
		std::vector<std::unordered_set<size_t>>& domFront;

		std::unordered_map<ILVarId, std::stack<ILVarId>> versionStacks;
		std::unordered_map<ILVarId, uint32_t> versionCounters;
		std::unordered_map<ILVarId, std::unordered_set<size_t>> defSites;

		uint64_t TopVersion(ILVarId varId)
		{
			if (versionStacks[varId].empty())
			{
				return varId;
			}

			return versionStacks[varId].top();
		}

		ILVarId MakeNewVersion(ILVarId varId, bool push = true)
		{
			uint32_t version = ++versionCounters[varId];
			ILVarId newVersion = MakeVersion(varId, version);
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

		void InsertPhiMeta(CFGNode& node, ILVarId varId, ILPhiId& phiIdOut);

		void Visit(size_t index, CFGNode& node, SSACFGContext& context) override;

		void VisitClose(size_t index, CFGNode& node, SSACFGContext& context) override;

	public:
		SSABuilder(ILContext* context) :
			CFGVisitor(context->GetCFG()),
			context(context),
			metadata(context->GetMetadata()),
			domTreeChildren(context->GetCFG().domTreeChildren),
			domFront(context->GetCFG().domFront)
		{
		}

		void Build();
	};
}

#endif