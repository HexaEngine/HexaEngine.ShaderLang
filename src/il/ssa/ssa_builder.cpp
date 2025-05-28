#include "ssa_builder.hpp"
#include "pch/std.hpp"

namespace HXSL
{
	void SSABuilder::Visit(size_t index, CFGNode& node, SSACFGContext& context)
	{
		auto& instructs = node.instructions;
		for (auto& instr : instructs)
		{
			if (instr.IsOp(OpCode_Phi))
			{
				auto var = cast<Variable>(instr.operandResult);
				auto newVersion = MakeNewVersion(var->varId);
				context.variables.push_back(var->varId);
				var->varId = newVersion;
				metadata.phiMetadata[cast<Phi>(instr.operandLeft)->phiId.value].varId = newVersion;
				continue;
			}

			if (auto var = dyn_cast<Variable>(instr.operandLeft))
			{
				var->varId = TopVersion(var->varId);
			}
			if (auto var = dyn_cast<Variable>(instr.operandRight))
			{
				var->varId = TopVersion(var->varId);
			}

			if (auto var = dyn_cast<Variable>(instr.operandResult))
			{
				auto varId = var->varId;
				uint64_t newVersion = MakeNewVersion(varId);
				context.variables.push_back(varId);
				var->varId = newVersion;
			}

			if (instr.IsOp(OpCode_StackAlloc))
			{
			}

			if (instr.IsOp(OpCode_OffsetAddress))
			{
			}

			if (instr.IsOp(OpCode_Load))
			{
			}

			if (instr.IsOp(OpCode_Store))
			{
			}
		}

		auto& phiMetadata = metadata.phiMetadata;
		for (auto succ : node.successors)
		{
			auto& succNode = cfg.GetNode(succ);
			size_t slot = succNode.GetPredecessorIndex(index);

			for (auto& instr : succNode.instructions)
			{
				if (!instr.IsOp(OpCode_Phi)) break;
				auto phiId = cast<Phi>(instr.operandLeft)->phiId;
				uint64_t varId = cast<Variable>(instr.operandResult)->varId.var.id;
				uint64_t version = TopVersion(varId);
				phiMetadata[phiId.value].params[slot] = version;
			}
		}
	}

	void SSABuilder::VisitClose(size_t index, CFGNode& node, SSACFGContext& context)
	{
		for (auto& varId : context.variables)
		{
			PopVersion(varId);
		}
	}

	void SSABuilder::InsertPhiMeta(CFGNode& node, ILVarId varId, ILPhiId& phiIdOut)
	{
		auto& globalMetadata = metadata;
		auto& phiMetadata = globalMetadata.phiMetadata;
		ILPhiId phiId = ILPhiId(static_cast<uint64_t>(phiMetadata.size()));
		phiMetadata.emplace_back();
		auto& meta = phiMetadata.back();
		meta.params.resize(node.predecessors.size());

		auto& var = globalMetadata.variables[varId.var.id];

		ILInstruction phi = ILInstruction(OpCode_Phi, context->MakePhi(phiId), context->MakeVariable(var));

		node.instructions.insert(node.instructions.begin(), phi);

		phiIdOut = phiId;
	}

	void SSABuilder::Build()
	{
		const size_t n = cfg.size();

		for (size_t i = 0; i < n; ++i)
		{
			for (auto& instr : cfg.GetNode(i).instructions)
			{
				auto var = dyn_cast<Variable>(instr.operandResult);
				if (!var || var->varId.var.temp)
				{
					continue;
				}
				defSites[var->varId].insert(i);
			}
		}

		std::unordered_map<uint64_t, std::unordered_set<size_t>> hasPhi;
		std::vector<std::vector<ILPhiId>> blockPhis(n);
		for (auto& p : defSites)
		{
			uint64_t varId = p.first;
			std::queue<size_t> wl;
			for (auto b : p.second) wl.push(b);

			while (!wl.empty())
			{
				size_t b = wl.front(); wl.pop();
				for (auto df : domFront[b])
				{
					if (hasPhi[varId].insert(df).second)
					{
						ILPhiId phiId;
						InsertPhiMeta(cfg.GetNode(df), varId, phiId);
						blockPhis[df].push_back(phiId);
						if (!p.second.count(df)) wl.push(df);
					}
				}
			}
		}

		Traverse(0);
		return;
	}
}