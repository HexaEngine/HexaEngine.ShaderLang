#ifndef MEMORY_SSA_BUILDER_HPP
#define MEMORY_SSA_BUILDER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	struct SSACFGContext
	{
		std::vector<uint64_t> variables;
	};

	struct MemoryLocation
	{
		Instruction* rootLocation;
		MemoryLocation* base;
		ILVarId varId;
		ILFieldAccess field;
	};

	class MemorySSABuilder : CFGVisitor<SSACFGContext>
	{
		ILMetadata& metadata;
		ControlFlowGraph& cfg;

		std::vector<std::vector<size_t>>& domTreeChildren;
		std::vector<std::unordered_set<size_t>>& domFront;
		std::unordered_map<uint64_t, std::unordered_set<size_t>> defSites;

		MemorySSABuilder(ILMetadata& metadata, ControlFlowGraph& cfg) : metadata(metadata), CFGVisitor(cfg), cfg(cfg), domTreeChildren(cfg.domTreeChildren), domFront(cfg.domFront)
		{
		}

		void Visit(size_t index, CFGNode& node, SSACFGContext& context) override
		{
		}

		void VisitClose(size_t index, CFGNode& node, SSACFGContext& context) override
		{
		}

		void ResolveLocation()
		{
		}

		void Build()
		{
		}
	};
}

#endif