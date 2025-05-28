#include "control_flow_graph.hpp"
#include "lt_dominator_tree.hpp"
#include "il/il_context.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	ControlFlowGraph::ControlFlowGraph(ILContext* context) : context(context), allocator(context->allocator), metadata(context->GetMetadata()) {}

	void ControlFlowGraph::Build(ILContainer& container, JumpTable& jumpTable)
	{
		nodes.clear();

		std::unordered_map<ILInstruction*, size_t> instrToNode;
		std::unordered_set<ILInstruction*> blockStarts;

		for (auto& loc : jumpTable.locations)
		{
			blockStarts.insert(loc);
		}

		size_t currentIdx = AddNode(ControlFlowType_Normal);

		while (auto instr = container.pop_front_move())
		{
			if (blockStarts.contains(instr))
			{
				currentIdx = AddNode(ControlFlowType_Normal);
			}

			auto next = instr->GetNext();
			auto& node = GetNode(currentIdx);
			auto newNode = node.instructions.append_move(instr);
			instrToNode.insert({ instr, currentIdx });

			switch (instr->opcode)
			{
			case OpCode_Jump:
			{
				node.type = ControlFlowType_Unconditional;
				currentIdx = AddNode(ControlFlowType_Normal);
			}
			break;
			case OpCode_JumpZero:
			case OpCode_JumpNotZero:
			{
				node.type = ControlFlowType_Conditional;
				currentIdx = AddNode(ControlFlowType_Normal);
			}
			break;
			case OpCode_Return:
			case OpCode_Discard:
				node.type = ControlFlowType_Exit;
				if (next)
				{
					currentIdx = AddNode(ControlFlowType_Normal);
				}
				break;
			}
		}

		for (auto& node : nodes)
		{
			if (node.instructions.empty())
				continue;

			auto& lastInstr = node.instructions.back();
			ILInstruction* term = &lastInstr;

			switch (lastInstr.opcode)
			{
			case OpCode_Jump:
			{
				if (auto label = dyn_cast<Label>(lastInstr.operandLeft))
				{
					auto target = jumpTable.GetLocation(label->label);
					auto targetNode = instrToNode[target];
					label->label = ILLabel(targetNode);
					Link(node.id, instrToNode[target]);
				}
				break;
			}
			case OpCode_JumpZero:
			case OpCode_JumpNotZero:
			{
				if (auto label = dyn_cast<Label>(lastInstr.operandLeft))
				{
					auto target = jumpTable.GetLocation(label->label);
					auto targetNode = instrToNode[target];
					label->label = ILLabel(targetNode);
					Link(node.id, targetNode);
				}

				Link(node.id, node.id + 1);
				break;
			}
			default:
			{
				Link(node.id, node.id + 1);
				if (term->GetNext())
				{
					Link(node.id, instrToNode[term->GetNext()]);
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

			auto phiIndex = cast<Phi>(instr.operandLeft)->phiId;
			auto& phiInputs = phiMetadata[phiIndex.value].params;

			if (removedPred >= phiInputs.size())
				continue;

			phiInputs.erase(phiInputs.begin() + removedPred);
			if (phiInputs.size() == 1)
			{
				instr.opcode = OpCode_Move;
				instr.operandLeft = context->MakeVariable(phiInputs[0]); //ILOperand(ILOperandKind_Variable, phiInputs[0]);
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

		std::swap(nodes[index], nodes[last]);

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
							auto label = dyn_cast<Label>(instr.operandLeft);
							if (label->label.value == last)
							{
								label->label = ILLabel(index);
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

		dst.instructions.prepend_move(src.instructions);

		RemoveNode(from);
	}
}