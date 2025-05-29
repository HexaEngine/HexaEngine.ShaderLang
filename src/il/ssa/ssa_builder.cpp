#include "ssa_builder.hpp"
#include "pch/std.hpp"

namespace HXSL
{
	void SSABuilder::Visit(size_t index, BasicBlock& node, SSACFGContext& context)
	{
		for (auto& instr : node)
		{
			if (auto phi = dyn_cast<PhiInstr>(&instr))
			{
				auto& var = phi->GetResult();
				auto newVersion = MakeNewVersion(var);
				context.variables.push_back(var);
				phi->SetResult(newVersion);
				continue;
			}

			for (auto operand : instr.GetOperands())
			{
				if (auto var = dyn_cast<Variable>(operand))
				{
					var->varId = TopVersion(var->varId);
				}
			}

			if (auto res = dyn_cast<ResultInstr>(&instr))
			{
				auto& varId = res->GetResult();
				ILVarId newVersion = MakeNewVersion(varId);
				context.variables.push_back(varId);
				res->SetResult(newVersion);
			}
		}

		for (auto succ : node.GetDependants())
		{
			auto& succNode = cfg.GetNode(succ);
			size_t slot = succNode.GetPredecessorIndex(index);

			for (auto& instr : succNode)
			{
				auto phi = dyn_cast<PhiInstr>(&instr);
				if (!phi) break;
				ILVarId varId = phi->GetResult();
				ILVarId version = TopVersion(varId);
				phi->GetOperand(slot) = this->context->MakeVariable(version);
			}
		}
	}

	void SSABuilder::VisitClose(size_t index, BasicBlock& node, SSACFGContext& context)
	{
		for (auto& varId : context.variables)
		{
			PopVersion(varId);
		}
	}

	void SSABuilder::InsertPhiMeta(BasicBlock& node, ILVarId varId)
	{
		auto& globalMetadata = metadata;
		auto& var = globalMetadata.variables[varId.var.id];
		auto incomingCount = node.GetDependencies().size();
		auto instr = node.InsertInstr(node.begin(), PhiInstr(context->allocator, var, incomingCount));
		globalMetadata.phiNodes.push_back(instr);
	}

	void SSABuilder::Build()
	{
		const size_t n = cfg.size();

		std::unordered_map<ILVarId, std::unordered_set<size_t>> defSites;
		for (size_t i = 0; i < n; ++i)
		{
			for (auto& instr : cfg.GetNode(i))
			{
				auto res = dyn_cast<ResultInstr>(&instr);
				if (!res) continue;
				auto& var = res->GetResult();
				if (var.temp())
				{
					continue;
				}
				defSites[var].insert(i);
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
						InsertPhiMeta(cfg.GetNode(df), varId);
						//blockPhis[df].push_back(phiId);
						if (!p.second.count(df)) wl.push(df);
					}
				}
			}
		}

		Traverse(0);
		return;
	}
}