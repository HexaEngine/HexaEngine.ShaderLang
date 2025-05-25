#include "control_flow_graph.hpp"
#include "lt_dominator_tree.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	void ControlFlowGraph::Build(ILContainer& container, JumpTable& jumpTable)
	{
		nodes.clear();

		auto size = container.size();

		std::vector<size_t> instrToNode;
		instrToNode.resize(size, -1);

		std::unordered_set<size_t> blockStarts;
		for (auto& loc : jumpTable.locations)
		{
			blockStarts.insert(loc);
		}

		size_t currentIdx = AddNode(ControlFlowType_Normal, 0);

		for (size_t i = 0; i < size; i++)
		{
			if (blockStarts.contains(i))
			{
				GetNode(currentIdx).SetTerminator(i - 1);
				currentIdx = AddNode(ControlFlowType_Normal, i);
			}

			auto& instr = container[i];

			auto& node = GetNode(currentIdx);

			node.AddInstr(instr);
			instrToNode[i] = currentIdx;

			switch (instr.opcode)
			{
			case OpCode_Jump:
			{
				node.type = ControlFlowType_Unconditional;
				node.SetTerminator(i);
				currentIdx = AddNode(ControlFlowType_Normal, i + 1);
			}
			break;
			case OpCode_JumpZero:
			case OpCode_JumpNotZero:
			{
				node.type = ControlFlowType_Conditional;
				node.SetTerminator(i);
				currentIdx = AddNode(ControlFlowType_Normal, i + 1);
			}
			break;
			case OpCode_Return:
			case OpCode_Discard:
				node.type = ControlFlowType_Exit;
				node.SetTerminator(i);
				if (i + 1 < size)
				{
					currentIdx = AddNode(ControlFlowType_Normal, i + 1);
				}
				break;
			}
		}
		GetNode(currentIdx).SetTerminator(size - 1);

		for (auto& node : nodes)
		{
			if (node.instructions.empty())
				continue;

			auto& lastInstr = node.instructions.back();
			size_t termIdx = node.terminator;

			switch (lastInstr.opcode)
			{
			case OpCode_Jump:
			{
				if (lastInstr.operandLeft.kind == ILOperandKind_Label)
				{
					size_t target = jumpTable.GetLocation(lastInstr.operandLeft.varId);
					auto targetNode = instrToNode[target];
					lastInstr.operandLeft.varId = targetNode;
					Link(node.id, instrToNode[target]);
				}
				break;
			}
			case OpCode_JumpZero:
			case OpCode_JumpNotZero:
			{
				if (lastInstr.operandLeft.kind == ILOperandKind_Label)
				{
					size_t target = jumpTable.GetLocation(lastInstr.operandLeft.varId);
					auto targetNode = instrToNode[target];
					lastInstr.operandLeft.varId = targetNode;
					Link(node.id, targetNode);
				}

				if (termIdx + 1 < size)
				{
					Link(node.id, instrToNode[termIdx + 1]);
				}
				break;
			}
			default:
			{
				if (termIdx + 1 < size)
				{
					Link(node.id, instrToNode[termIdx + 1]);
				}
				break;
			}
			}
		}

		RebuildDomTree();
	}

	void ControlFlowGraph::RebuildDomTree()
	{
		LTDominatorTree tree = LTDominatorTree(*this);
		idom = tree.Compute(0);
		const size_t n = nodes.size();

		domTreeChildren.clear();
		domTreeChildren.resize(n);
		for (size_t i = 0; i < n; i++)
		{
			if (idom[i] != i && idom[i] != static_cast<size_t>(-1))
			{
				domTreeChildren[idom[i]].push_back(i);
			}
		}

		domFront = tree.ComputeDominanceFrontiers(idom, domTreeChildren);
	}

	void ControlFlowGraph::UpdatePhiInputs(size_t removedPred, size_t targetBlock)
	{
		auto& phiMetadata = metadata.phiMetadata;
		auto& targetNode = nodes[targetBlock];

		for (auto& instr : targetNode.instructions)
		{
			if (instr.opcode != OpCode_Phi)
				break;

			uint64_t phiIndex = instr.operandLeft.varId;
			auto& phiInputs = phiMetadata[phiIndex].params;

			if (removedPred >= phiInputs.size())
				continue;

			phiInputs.erase(phiInputs.begin() + removedPred);
			if (phiInputs.size() == 1)
			{
				instr.opcode = OpCode_Move;
				instr.operandLeft = ILOperand(ILOperandKind_Variable, phiInputs[0]);
			}
		}
	}

	void ControlFlowGraph::Unlink(size_t from, size_t to)
	{
		nodes[from].RemoveSuccessor(to);
		nodes[to].RemovePredecessor(from);
		UpdatePhiInputs(to, from);
	}

	void ControlFlowGraph::RemoveNode(size_t index)
	{
		auto& node = nodes[index];
		for (auto& pred : node.predecessors)
		{
			nodes[pred].RemoveSuccessor(index);
		}
		for (auto& succs : node.successors)
		{
			UpdatePhiInputs(index, succs);
			nodes[succs].RemovePredecessor(index);
		}

		auto last = nodes.size() - 1;
		if (last == index)
		{
			nodes.pop_back();
			return;
		}

		std::swap(node, nodes[last]);

		auto& swapped = nodes[index];
		swapped.id = index;

		for (auto pred : swapped.predecessors)
		{
			for (auto& s : nodes[pred].successors)
			{
				if (s == last)
				{
					s = index;
					for (auto& instr : nodes[pred].instructions)
					{
						if (instr.opcode == OpCode_Jump || instr.opcode == OpCode_JumpNotZero || instr.opcode == OpCode_JumpZero)
						{
							if (instr.operandLeft.label == last)
							{
								instr.operandLeft.label = index;
							}
						}
					}
				}
			}
		}

		for (auto succs : swapped.successors)
		{
			for (auto& p : nodes[succs].predecessors)
			{
				if (p == last) p = index;
			}
		}

		nodes.pop_back();
	}

	void ControlFlowGraph::MergeNodes(size_t from, size_t to)
	{
		auto& src = nodes[from];
		auto& dst = nodes[to];

		for (auto& pred : src.predecessors)
		{
			nodes[pred].RemoveSuccessor(from);
			Link(pred, to);
		}
		src.predecessors.clear();

		for (auto& succs : src.successors)
		{
			nodes[succs].RemovePredecessor(from);
			Link(to, succs);
		}
		src.successors.clear();

		dst.instructions.insert(dst.instructions.begin(), src.instructions.begin(), src.instructions.end());

		src.instructions.clear();
		dst.startInstr = src.startInstr;

		RemoveNode(from);
	}
}