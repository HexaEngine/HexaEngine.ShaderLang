#ifndef LOOP_TREE_HPP
#define LOOP_TREE_HPP

#include "control_flow_graph.hpp"

namespace HXSL
{
	namespace Backend
	{
		class LoopTree;

		class LoopNode
		{
			friend class LoopTree;
			BasicBlock* header = nullptr;
			BasicBlock* preHeader = nullptr;
			std::vector<BasicBlock*> blocks;
			std::unordered_set<BasicBlock*> latches;
			std::unordered_set<BasicBlock*> exits;
			
			LoopNode* parent = nullptr;
			std::vector<LoopNode*> children;
			size_t depth = 0;

		public:
			LoopNode() = default;
			LoopNode(BasicBlock* header) : header(header) {}

			BasicBlock* GetHeader() const { return header; }
			BasicBlock* GetPreHeader() const { return preHeader; }
			const std::vector<BasicBlock*>& GetBlocks() const { return blocks; }
			const std::unordered_set<BasicBlock*>& GetLatches() const { return latches; }
			const std::unordered_set<BasicBlock*>& GetExits() const { return exits; }
			
			LoopNode* GetParent() const { return parent; }
			const std::vector<LoopNode*>& GetChildren() const { return children; }
			size_t GetDepth() const { return depth; }

			std::string ToString() const;

			void UpdateDepth(size_t newDepth)
			{
				depth = newDepth;
				for (auto* child : children)
				{
					child->UpdateDepth(newDepth + 1);
				}
			}
		};

		class LoopTree
		{
			ControlFlowGraph& cfg;
			std::vector<uptr<LoopNode>> nodes;
			dense_map<BasicBlock*, LoopNode*> headerToNode;
			dense_map<BasicBlock*, LoopNode*> blockToNode;

			LoopNode* CreateNode(BasicBlock* header)
			{
				nodes.emplace_back(make_uptr<LoopNode>(header));
				return nodes.back().get();
			}

			void LinkNode(LoopNode* parent, LoopNode* child)
			{
				HXSL_ASSERT(child->parent == nullptr, "Child node already has a parent.");
				child->parent = parent;
				child->UpdateDepth(parent->depth + 1);
				parent->children.push_back(child);
			}

			void UnlinkNode(LoopNode* child)
			{
				auto* parent = child->parent;
				if (parent == nullptr) return;
				auto& siblings = parent->children;
				siblings.erase(std::remove(siblings.begin(), siblings.end(), child), siblings.end());
				child->parent = nullptr;
				child->depth = 0;
			}

			void BuildLoop(LoopNode* node);

		public:
			LoopTree(ControlFlowGraph& cfg) : cfg(cfg) {}
			void Build();
			void Clear() { nodes.clear(); }

			void Print() const;
		};
	}
}

#endif