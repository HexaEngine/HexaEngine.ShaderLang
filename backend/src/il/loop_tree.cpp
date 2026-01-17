#include "il/loop_tree.hpp"

namespace HXSL
{
	namespace Backend
	{
		std::string LoopNode::ToString() const
		{
			std::string result = "LoopNode(Header: " + std::to_string(header->GetId()) + ", Depth: " + std::to_string(depth) + ")\n";
			if (preHeader)
			{
				result += "  PreHeader: " + std::to_string(preHeader->GetId()) + "\n";
			}
			result += "  Blocks: ";
			for (auto* block : blocks)
			{
				result += std::to_string(block->GetId()) + " ";
			}
			result += "\n";
			result += "  Latches: ";
			for (auto* latch : latches)
			{
				result += std::to_string(latch->GetId()) + " ";
			}
			result += "\n";
			result += "  Exits: ";
			for (auto* exit : exits)
			{
				result += std::to_string(exit->GetId()) + " ";
			}
			result += "\n";
			return result;
		}


		void LoopTree::BuildLoop(LoopNode* node)
		{
			auto header = node->header;
			std::unordered_set<BasicBlock*> bodySet;
			bodySet.insert(header);
			
			for (auto predIdx : header->GetPredecessors())
			{
				auto* pred = cfg.GetNode(predIdx).get();
				if (!node->latches.contains(pred))
				{
					if (node->preHeader != nullptr)
					{
						node->preHeader = nullptr;
						break;
					}
					node->preHeader = pred;
				}
			}

			std::queue<BasicBlock*> worklist;
			for (auto latch : node->latches)
			{
				if (bodySet.insert(latch).second)
				{
					worklist.push(latch);
				}
			}

			while (!worklist.empty())
			{
				auto* current = worklist.front();
				worklist.pop();

				// Add all predecessors (except don't go past header)
				for (auto predIdx : current->GetPredecessors())
				{
					auto* pred = cfg.GetNode(predIdx).get();

					if (pred == header)
						continue;  // Don't go past header

					// Add to body if not already there
					if (bodySet.insert(pred).second)
					{
						worklist.push(pred);
					}
				}
			}

			for (auto* block : bodySet)
			{
				node->blocks.push_back(block);
			}

			std::sort(node->blocks.begin(), node->blocks.end(),
				[&](BasicBlock* a, BasicBlock* b)
				{
					return a->GetId() < b->GetId();
				});

			for (auto* bodyBlock : node->blocks)
			{
				for (auto succIdx : bodyBlock->GetSuccessors())
				{
					auto* succ = cfg.GetNode(succIdx).get();

					// Exit block = successor not in loop body
					if (bodySet.find(succ) == bodySet.end())
					{
						node->exits.insert(succ);
					}
				}
			}
		}

		void LoopTree::Build()
		{
			Clear();

			auto& cfgNodes = cfg.GetNodes();
			for (size_t n = 0; n < cfgNodes.size(); n++)
			{
				for (size_t succ : cfgNodes[n]->GetSuccessors())
				{
					if (cfg.Dominates(succ, n))
					{
						auto header = cfgNodes[succ].get();
						auto latch = cfgNodes[n].get();

						auto it = headerToNode.find(header);
						LoopNode* loopNode;
						if (it == headerToNode.end())
						{
							loopNode = CreateNode(header);
							headerToNode.insert({ header, loopNode });
						}
						else
						{
							loopNode = it->second;
						}
						loopNode->latches.insert(latch);
					}
				}
			}

			for (auto& node : nodes)
			{
				BuildLoop(node.get());
			}


			for (auto& node : nodes)
			{
				for (auto* block : node->blocks)
				{
					auto it = headerToNode.find(block);
					if (it != headerToNode.end() && it->second != node.get())
					{
						auto* child = it->second;
						auto* parent = node.get();
						auto* oldParent = child->parent;
						if (oldParent != nullptr && parent->blocks.size() < oldParent->blocks.size())
						{
							UnlinkNode(child);
							LinkNode(parent, child);
						}
						else if (oldParent == nullptr)
						{
							LinkNode(parent, child);
						}
					}
				}
			}
		}

		void LoopTree::Print() const
		{
			for (const auto& node : nodes)
			{
				std::cout << node->ToString() << std::endl;
			}
		}
	}
}