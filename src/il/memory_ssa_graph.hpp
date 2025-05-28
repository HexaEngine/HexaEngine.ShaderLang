#ifndef MEMORY_SSA_GRAPH_HPP
#define MEMORY_SSA_GRAPH_HPP

#include "pch/std.hpp"
#include "il_instruction.hpp"

namespace HXSL
{
	enum MemorySSANodeType : char
	{
		MemoryNodeType_Dead,
		MemoryNodeType_Use,
		MemoryNodeType_Def,
		MemoryNodeType_Phi
	};

	template <typename MemLoc, typename IndexType = uint32_t, typename BlockIdType = uint32_t, typename InstrIdType = uint32_t>
	class MemorySSAGraph
	{
	public:
		static constexpr IndexType INVALID_INDEX = std::numeric_limits<IndexType>::max();

		struct MemorySSANode
		{
			MemorySSANodeType type;
			union
			{
				IndexType parent;
				IndexType phiMetadataIdx;
			};
			std::vector<IndexType> children;
			BlockIdType blockId;
			InstrIdType instrId;

			MemorySSANode(MemorySSANodeType type, IndexType parent, const BlockIdType& blockId, const InstrIdType& instrId)
				: type(type), parent(parent), blockId(blockId), instrId(instrId)
			{
			}
		};

	private:
		std::vector<MemorySSANode> nodes;
		std::vector<std::vector<IndexType>> phiMetadata;
		MemLoc memoryLocation;

		IndexType freeIndex = 0;
		IndexType freeCount = 0;

		IndexType freeIndexPhi = 0;
		IndexType freeCountPhi = 0;

		IndexType AddNode(MemorySSANodeType type, IndexType parent, const BlockIdType& blockId, const InstrIdType& instrId)
		{
			if (freeCount != 0)
			{
				auto idx = freeIndex;
				if (--freeCount != 0)
				{
					const size_t n = nodes.size();
					for (IndexType i = freeIndex + 1; i < n; i++)
					{
						if (nodes[i].type == MemoryNodeType_Dead)
						{
							freeIndex = i;
							break;
						}
					}
				}
				else
				{
					freeIndex = 0;
				}

				auto node = &nodes[idx];
				new(node) MemorySSANode(type, parent, std::move(blockId), std::move(instrId));
				return idx;
			}
			else
			{
				auto idx = nodes.size();
				nodes.emplace_back(type, parent, std::move(blockId), std::move(instrId));
				return static_cast<IndexType>(idx);
			}
		}

		void RemoveNode(IndexType index)
		{
			auto last = nodes.size() - 1;
			if (index == last)
			{
				nodes.pop_back();
				if (nodes.size() == 0)
				{
					freeIndex = 0;
					freeCount = 0;
				}
				return;
			}

			freeIndex = std::min(freeIndex, index);
			++freeCount;
			auto node = &nodes[index];
			node->~MemorySSANode();
			node->type = MemoryNodeType_Dead;
		}

		void MergeNode(IndexType idx, IndexType parentIdx)
		{
			auto& node = nodes[idx];
			auto& parent = nodes[parentIdx];
			std::erase(parent.children, idx);
			for (auto& c : node.children)
			{
				auto& child = nodes[c];
				if (child.type == MemoryNodeType_Phi)
				{
					auto& phi = phiMetadata[child.phiMetadataIdx];
					if (parent.blockId == node.blockId)
					{
						std::replace(phi.begin(), phi.end(), idx, parentIdx);
						parent.children.push_back(c);
					}
					else
					{
						std::erase(phi, idx);
						if (phi.size() == 0)
						{
							RemoveNode(c);
						}
					}
				}
				else
				{
					nodes[c].parent = node.parent;
					parent.children.push_back(c);
				}
			}
		}

		IndexType AllocPhi(size_t size)
		{
			const size_t n = phiMetadata.size();
			if (freeCountPhi != 0)
			{
				auto idx = freeIndexPhi;
				if (--freeCountPhi != 0)
				{
					for (IndexType i = freeIndexPhi + 1; i < n; ++i)
					{
						if (phiMetadata[i].size() == 0)
						{
							freeIndexPhi = i;
							break;
						}
					}
				}
				else
				{
					freeIndexPhi = 0;
				}

				phiMetadata[idx].resize(size);
				return idx;
			}

			std::vector<IndexType> phi;
			phi.resize(size);
			phiMetadata.emplace_back(std::move(phi));
			return n;
		}

		void FreePhi(IndexType idx)
		{
			freeIndexPhi = std::min(freeIndexPhi, idx);
			++freeCountPhi;
			phiMetadata[idx] = {};
		}

	public:
		MemorySSAGraph(const MemLoc& memLoc) : memoryLocation(memLoc) {}

		MemLoc& GetMemoryLocation() { return memoryLocation; }

		MemorySSANode& GetNode(IndexType idx) { return nodes[idx]; }

		std::vector<IndexType>& GetPhi(IndexType phiIdx) { return phiMetadata[phiIdx]; }

		std::vector<IndexType>& GetPhiFromNode(IndexType nodeIdx) { return phiMetadata[nodes[nodeIdx].phiMetadataIdx]; }

		IndexType AddUse(IndexType def, const BlockIdType& blockId, const InstrIdType& instrId)
		{
			if (def == INVALID_INDEX) throw std::runtime_error("MemoryDef invalid");
			auto idx = AddNode(MemoryNodeType_Use, def, blockId, instrId);
			nodes[def].children.push_back(idx);
			return idx;
		}

		IndexType AddDef(IndexType def, const BlockIdType& blockId, const InstrIdType& instrId)
		{
			auto idx = AddNode(MemoryNodeType_Def, def, blockId, instrId);
			if (def != INVALID_INDEX)
			{
				nodes[def].children.push_back(idx);
			}
			return idx;
		}

		IndexType AddPhi(const InstrIdType& instrId, const BlockIdType& blockId, size_t predecessors)
		{
			auto phiIdx = AllocPhi(predecessors);
			auto idx = AddNode(MemoryNodeType_Phi, phiIdx, blockId, instrId);
			return idx;
		}

		void LinkPhi(IndexType phiIdx, size_t slot, IndexType predIdx)
		{
			auto& predNode = nodes[predIdx];
			if (predNode.type != MemoryNodeType_Def) throw std::runtime_error("Predecessor must be MemoryDef");
			auto& phiNode = nodes[phiIdx];
			if (phiNode.type != MemoryNodeType_Phi) throw std::runtime_error("Phi must be MemoryPhi");
			auto& phi = phiMetadata[phiNode.phiMetadataIdx];
			phi[slot] = predIdx;
			predNode.children.push_back(phiIdx);
		}

		void Remove(IndexType idx)
		{
			auto& node = nodes[idx];
			if (node.type == MemoryNodeType_Use)
			{
				auto& parent = nodes[node.parent];
				std::erase(parent.children, idx);
			}
			else if (node.type == MemoryNodeType_Def)
			{
				MergeNode(idx, node.parent);
			}
			else 
			{
				auto& phi = phiMetadata[node.phiMetadataIdx];
				if (phi.size() > 1)
				{
					throw std::runtime_error("Cannot delete a MemoryPhi that has more than 1 incoming node");
				}
				if (phi.size() == 0)
				{
					for(auto& c : node.children)
					{
						Remove(c);
					}
				}
				else
				{
					MergeNode(idx, phi[0]);
				}

				FreePhi(node.parent);
			}

			RemoveNode(idx);
		}
	};
}

#endif