#ifndef CONTROL_FLOW_GRAPH_HPP
#define CONTROL_FLOW_GRAPH_HPP

#include "pch/ast.hpp"
#include "instruction.hpp"
#include "il_text.hpp"

#include "il_container.hpp"
#include "il_metadata.hpp"
#include "jump_table.hpp"
#include "graph_base.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILContext;

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

		struct BasicBlock : public GraphNode<BasicBlock>
		{
		public:
			using instr_iterator = ilist<Instruction>::iterator;
			using const_instr_iterator = ilist<Instruction>::const_iterator;
			using reverse_instr_iterator = ilist<Instruction>::reverse_iterator;
			using const_reverse_instr_iterator = ilist<Instruction>::const_reverse_iterator;
		private:
			friend class ControlFlowGraph;
			size_t id;
			ILContext* parent;
			ControlFlowType type;
			ilist<Instruction> instructions;
			std::vector<size_t> predecessors;
			std::vector<size_t> successors;

		public:
			BasicBlock(BumpAllocator& allocator, size_t id, ILContext* parent, ControlFlowType type) : id(id), parent(parent), type(type), instructions({ allocator }) {}

			ILContext* GetParent() const { return parent; }

			void SetType(ControlFlowType value) noexcept { type = value; }
			ControlFlowType GetType() const noexcept { return type; }

			const std::vector<size_t>& GetDependencies() const { return predecessors; }
			const std::vector<size_t>& GetDependants() const { return successors; }
			const std::vector<size_t>& GetPredecessors() const { return predecessors; }
			const std::vector<size_t>& GetSuccessors() const { return successors; }
			size_t NumPredecessors() const noexcept { return predecessors.size(); }
			size_t NumSuccessors() const noexcept { return successors.size(); }
			bool IsPredecessorsEmpty() const noexcept { return predecessors.empty(); }
			bool IsSuccessorsEmpty() const noexcept { return successors.empty(); }

			const ilist<Instruction>& GetInstructions() const { return instructions; }

			instr_iterator begin() { return instructions.begin(); }
			instr_iterator end() { return instructions.end(); }
			const_instr_iterator begin() const { return instructions.begin(); }
			const_instr_iterator end() const { return instructions.end(); }
			const_instr_iterator cbegin() const { return instructions.begin(); }
			const_instr_iterator cend() const { return instructions.end(); }

			reverse_instr_iterator rbegin() { return instructions.rbegin(); }
			reverse_instr_iterator rend() { return instructions.rend(); }
			const_reverse_instr_iterator rbegin() const { return instructions.rbegin(); }
			const_reverse_instr_iterator rend() const { return instructions.rend(); }
			const_reverse_instr_iterator rcbegin() const { return instructions.rbegin(); }
			const_reverse_instr_iterator rcend() const { return instructions.rend(); }

			void AddInstr(Instruction* instr)
			{
				instr->SetParent(this);
				instructions.append_move(instr);
			}

			template<typename U>
			U* InsertInstr(const instr_iterator& it, const U& instr)
			{
				auto& allocator = instructions.get_allocator();
				auto res = instructions.insert(it, allocator.Alloc<U>(instr));
				res->SetParent(this);
				return static_cast<U*>(res);
			}

			template<typename U>
			U* InsertInstr(const instr_iterator& it, U&& instr)
			{
				auto& allocator = instructions.get_allocator();
				auto res = instructions.insert(it, allocator.Alloc<U>(std::forward<U>(instr)));
				res->SetParent(this);
				return static_cast<U*>(res);
			}

			void RemoveInstr(Instruction* instr)
			{
				instr->SetParent(nullptr);
				instructions.remove(instr);
			}

			template<typename T, typename... Operands>
			Instruction* ReplaceInstr(Instruction* instr, ILOpCode opcode, ILVarId result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				auto& allocator = instructions.get_allocator();
				OperandFactory factory{ allocator };
				auto loc = instr->GetLocation();
				auto res = instructions.emplace_replace<T>(instr, allocator, opcode, result, factory(std::forward<Operands>(operands))...);
				res->SetLocation(loc);
				res->SetParent(this);
				return res;
			}

			template<typename T, typename... Operands>
			Instruction* ReplaceInstrNO(Instruction* instr, ILOpCode opcode, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				auto& allocator = instructions.get_allocator();
				OperandFactory factory{ allocator };
				auto loc = instr->GetLocation();
				auto res = instructions.emplace_replace<T>(instr, allocator, opcode, factory(std::forward<Operands>(operands))...);
				res->SetLocation(loc);
				res->SetParent(this);
				return res;
			}

			template<typename T, typename... Operands>
			Instruction* ReplaceInstrO(Instruction* instr, ILVarId result, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				auto& allocator = instructions.get_allocator();
				OperandFactory factory{ allocator };
				auto loc = instr->GetLocation();
				auto res = instructions.emplace_replace<T>(instr, allocator, result, factory(std::forward<Operands>(operands))...);
				res->SetLocation(loc);
				res->SetParent(this);
				return res;
			}

			template<typename T, typename... Operands>
			Instruction* ReplaceInstrONO(Instruction* instr, Operands&&... operands)
			{
				static_assert(std::is_base_of_v<Instruction, T>, "T must derive from Instruction");
				auto& allocator = instructions.get_allocator();
				OperandFactory factory{ allocator };
				auto loc = instr->GetLocation();
				auto res = instructions.emplace_replace<T>(instr, allocator, factory(std::forward<Operands>(operands))...);
				res->SetLocation(loc);
				res->SetParent(this);
				return res;
			}

			void InstructionsTrimEnd(Instruction* instr)
			{
				instructions.trim_end(instr);
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
		};

		class ControlFlowGraph : public GraphBase<BasicBlock>
		{
			friend class LTDominatorTree;
			ILContext* context;
		public:
			BumpAllocator& allocator;
			ILMetadata& metadata;
			std::vector<size_t> idom;
			std::vector<std::vector<size_t>> domTreeChildren;
			std::vector<std::unordered_set<size_t>> domFront;

			ControlFlowGraph(ILContext* context);

			void Build(ILContainer& container, JumpTable& jumpTable);

			void RebuildDomTree();

			void UpdatePhiInputs(size_t removedPred, size_t targetBlock);

			size_t AddNode(ControlFlowType type)
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
				nodes.emplace_back(allocator, index, context, type);
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

			void Print() const
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

			virtual void Visit(size_t index, BasicBlock& node, TContext& context) = 0;

			virtual void VisitClose(size_t index, BasicBlock& node, TContext& context) {}

		public:
			void Traverse(size_t entryIdx = 0)
			{
				if (cfg.empty()) return;

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
					walkStack.push({ currentIdx, true, std::move(context) });

					auto& children = domTreeChildren[currentIdx];
					if (!children.empty())
					{
						for (auto it = children.begin(); it != children.end(); ++it)
						{
							walkStack.push({ *it , false, {} });
						}
					}
				}
			}
		};
	}
}

#endif