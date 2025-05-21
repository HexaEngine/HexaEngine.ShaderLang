#include "ssa_builder.hpp"
#include "pch/std.hpp"

namespace HXSL
{
	static void MapTempRegisterOp(const std::unordered_map<ILRegister, ILRegister>& r2r, ILOperand& op)
	{
		auto it = r2r.find(op.reg);
		if (it != r2r.end())
		{
			op.reg = it->second;
		}
	}

	void SSABuilder::MapTempRegister(ILInstruction& instr)
	{
		if (instr.operandLeft.IsReg())
		{
			MapTempRegisterOp(r2r, instr.operandLeft);
		}
		if (instr.operandRight.IsReg())
		{
			MapTempRegisterOp(r2r, instr.operandRight);
		}
		if (instr.operandResult.IsReg())
		{
			r2r[instr.operandResult.reg] = currentTemp;
			instr.operandResult.reg = currentTemp;
			currentTemp.id++;
		}
	}

	void SSABuilder::Visit(size_t index, CFGNode& node, SSACFGContext& context)
	{
		auto& instructs = node.instructions;
		const size_t n = node.instructions.size();
		for (size_t i = 0; i < n; ++i)
		{
			auto& instr = node.instructions[i];

			if (instr.IsOp(OpCode_Phi))
			{
				uint64_t varId = instr.operandResult.varId;
				uint64_t newVersion = MakeNewVersion(varId);
				context.variables.push_back(varId);
				instr.operandResult.varId = newVersion;
				continue;
			}

			if (instr.operandResult.IsVar())
			{
				auto varId = instr.operandResult.varId;
				uint64_t newVersion = MakeNewVersion(varId);
				context.variables.push_back(varId);
				instr.operandResult.varId = newVersion;
			}

			if (instr.operandLeft.IsVar())
			{
				instr.operandLeft.varId = TopVersion(instr.operandLeft.varId);
			}
			if (instr.operandRight.IsVar())
			{
				instr.operandRight.varId = TopVersion(instr.operandRight.varId);
			}

			MapTempRegister(instr);
		}

		DiscardMarkedInstructs(node);

		auto& phiMetadata = builder.GetMetadata().phiMetadata;
		for (auto succ : node.successors)
		{
			auto& succNode = cfg.GetNode(succ);
			size_t slot = succNode.GetPredecessorIndex(index);

			for (size_t i = 0; i < succNode.instructions.size(); ++i)
			{
				auto& instr = succNode.instructions[i];
				if (!instr.IsOp(OpCode_Phi)) break;
				size_t phiId = instr.operandLeft.varId;
				uint64_t varId = instr.operandResult.varId & SSA_VARIABLE_MASK;
				uint64_t version = TopVersion(varId);
				phiMetadata[phiId].params[slot] = version;
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

	void SSABuilder::InsertPhiMeta(CFGNode& node, uint64_t varId, size_t& phiIdOut)
	{
		auto& globalMetadata = builder.GetMetadata();
		auto& phiMetadata = globalMetadata.phiMetadata;
		size_t phiId = phiMetadata.size();
		phiMetadata.emplace_back();
		auto& meta = phiMetadata.back();
		meta.params.resize(node.predecessors.size());

		auto& var = globalMetadata.variables[varId];

		ILInstruction phi = ILInstruction(OpCode_Phi, ILOperand(ILOperandKind_Phi, phiId), var.AsTypeOperand(), var.AsOperand());

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
				if (instr.operandResult.IsVar())
				{
					defSites[instr.operandResult.varId].insert(i);
				}
			}
		}

		std::unordered_map<uint64_t, std::unordered_set<size_t>> hasPhi;
		std::vector<std::vector<size_t>> blockPhis(n);
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
						size_t phiId = 0;
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