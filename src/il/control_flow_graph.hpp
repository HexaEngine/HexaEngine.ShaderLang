#ifndef CONTROL_FLOW_GRAPH_HPP
#define CONTROL_FLOW_GRAPH_HPP

#include "pch/ast.hpp"
#include "il/il_instruction.hpp"
#include "il/il_container.hpp"
#include "il/il_builder.hpp"
#include "graph_base.hpp"

namespace HXSL
{
	enum ControlFlowType
	{
		ControlFlowType_Unknown,
		ControlFlowType_Normal,
		ControlFlowType_Conditional,
		ControlFlowType_Unconditional,
		ControlFlowType_Exit
	};

	static std::string ToString(ControlFlowType type)
	{
		switch (type)
		{
		case ControlFlowType_Normal:
			return "Normal";
		case ControlFlowType_Conditional:
			return "Conditional";
		case ControlFlowType_Unconditional:
			return "Unconditional";
		case ControlFlowType_Exit:
			return "Exit";
		default:
			return "Unknown";
			break;
		}
	}

	struct CFGNode : public GraphNode<CFGNode>
	{
		size_t id;
		size_t startInstr;
		ControlFlowType type;
		std::vector<ILInstruction> instructions;
		std::vector<size_t> predecessors;
		std::vector<size_t> successors;
		size_t terminator;

		CFGNode(size_t id, size_t startInstr, ControlFlowType type) : id(id), startInstr(startInstr), type(type), terminator(-1) {}

		const std::vector<size_t>& GetDependencies() const
		{
			return predecessors;
		}

		const std::vector<size_t>& GetDependants() const
		{
			return successors;
		}

		void AddInstr(const ILInstruction& instr)
		{
			instructions.push_back(instr);
		}

		void AddPredecessor(size_t predId)
		{
			predecessors.push_back(predId);
		}

		void AddSuccessor(size_t succsId)
		{
			successors.push_back(succsId);
		}

		void RemovePredecessor(size_t predId)
		{
			auto it = std::find(predecessors.begin(), predecessors.end(), predId);
			if (it == predecessors.end()) return;
			predecessors.erase(it);
		}

		void RemoveSuccessor(size_t succsId)
		{
			auto it = std::find(successors.begin(), successors.end(), succsId);
			if (it == successors.end()) return;
			successors.erase(it);
		}

		size_t GetPredecessorIndex(size_t predId)
		{
			auto it = std::find(predecessors.begin(), predecessors.end(), predId);
			if (it == predecessors.end()) return -1;
			return it - predecessors.begin();
		}

		void SetTerminator(size_t term)
		{
			terminator = term;
		}
	};

	class ControlFlowGraph
	{
	public:
		ILBuilder& builder;
		std::vector<CFGNode> nodes;
		std::vector<size_t> idom;
		std::vector<std::vector<size_t>> domTreeChildren;
		std::vector<std::unordered_set<size_t>> domFront;

		ControlFlowGraph(ILBuilder& builder) : builder(builder) {}

		void Build();

		void RebuildDomTree();

		void UpdatePhiInputs(size_t removedPred, size_t targetBlock);

		size_t size() const { return nodes.size(); }

		const std::vector<CFGNode>& GetNodes() const { return nodes; }

		CFGNode& GetNode(size_t index)
		{
			return nodes[index];
		}

		size_t AddNode(ControlFlowType type, size_t startInstr)
		{
			if (!nodes.empty())
			{
				auto& last = nodes.back();
				if (last.successors.empty() && last.instructions.empty())
				{
					last.type = type;
					return last.id;
				}
			}

			auto index = nodes.size();
			CFGNode node = CFGNode(index, startInstr, type);
			nodes.push_back(node);
			return index;
		}

		void Link(size_t from, size_t to)
		{
			auto& fromNode = nodes[from];
			if (fromNode.type == ControlFlowType_Exit)
			{
				return;
			}
			fromNode.AddSuccessor(to);
			auto& toNode = nodes[to];
			toNode.AddPredecessor(from);
		}

		void Unlink(size_t from, size_t to);

		void RemoveNode(size_t index);

		void MergeNodes(size_t from, size_t to);

		void Print(const ILMetadata& metadata) const
		{
			for (const auto& node : nodes)
			{
				std::cout << "Node " << node.id << " [" << ToString(node.type) << "]\n";

				std::cout << "  Instructions:\n";
				for (const auto& instr : node.instructions)
				{
					std::cout << "    " << ToString(instr, metadata) << "\n";
				}

				std::cout << "  Predecessors: ";
				for (size_t pred : node.predecessors)
				{
					std::cout << pred << " ";
				}
				std::cout << "\n";

				std::cout << "  Successors: ";
				for (size_t succ : node.successors)
				{
					std::cout << succ << " ";
				}
				std::cout << "\n";

				std::cout << "  Terminator: " << node.terminator << "\n";
				std::cout << "\n";
			}
		}
	};

	struct EmptyCFGContext
	{
	};

	template<typename TContext = EmptyCFGContext>
	class CFGVisitor
	{
	protected:
		ControlFlowGraph& cfg;

		CFGVisitor(ControlFlowGraph& cfg) : cfg(cfg)
		{
		}

		virtual void Visit(size_t index, CFGNode& node, TContext& context) = 0;

		virtual void VisitClose(size_t index, CFGNode& node, TContext& context) {}

	public:
		void Traverse(size_t entryIdx = 0)
		{
			auto& domTreeChildren = cfg.domTreeChildren;

			std::stack<std::tuple<size_t, bool, TContext>> walkStack;
			walkStack.push({ entryIdx , false, {} });

			while (!walkStack.empty())
			{
				auto [currentIdx, closing, context] = std::move(walkStack.top());
				walkStack.pop();
				auto& node = cfg.GetNode(currentIdx);

				if (closing)
				{
					VisitClose(currentIdx, node, context);
					continue;
				}

				Visit(currentIdx, node, context);

				auto& children = domTreeChildren[currentIdx];
				if (!children.empty())
				{
					walkStack.push({ currentIdx, true, std::move(context) });

					for (auto it = children.rbegin(); it != children.rend(); ++it)
					{
						walkStack.push({ *it , false, {} });
					}
				}
			}
		}
	};
}

#endif