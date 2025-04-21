#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "nodes.hpp"
#include "compilation.hpp"
#include "stream.h"
#include "string_pool.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <stack>
#include <deque>

namespace HXSL
{
	class SymbolMetadata
	{
	public:
		SymbolType SymbolType;
		HXSLSymbolScopeType Scope;
		HXSLAccessModifier AccessModifier;
		size_t Size;
		HXSLSymbolDef* Declaration;

		SymbolMetadata() : SymbolType(HXSLSymbolType_Unknown), Scope(HXSLSymbolScopeType_Global), AccessModifier(HXSLAccessModifier_Private), Size(0), Declaration(nullptr)
		{
		}

		SymbolMetadata(const SymbolType& symbolType, const HXSLSymbolScopeType& scope, const HXSLAccessModifier& modifier, const size_t& size) : SymbolType(symbolType), Scope(scope), AccessModifier(modifier), Size(size), Declaration(nullptr)
		{
		}

		SymbolMetadata(const SymbolType& symbolType, const HXSLSymbolScopeType& scope, const HXSLAccessModifier& modifier, const size_t& size, HXSLSymbolDef* declaration) : SymbolType(symbolType), Scope(scope), AccessModifier(modifier), Size(size), Declaration(declaration)
		{
		}

		void Write(HXSLStream& stream) const;

		void Read(HXSLStream& stream, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes, StringPool& container);
	};

	struct SymbolTableNode
	{
		std::unique_ptr<std::string> Name;
		std::unordered_map<TextSpan, size_t, TextSpanHash, TextSpanEqual> Children;
		std::shared_ptr<SymbolMetadata> Metadata;
		size_t Depth;
		size_t ParentIndex;

		SymbolTableNode() : Depth(0), Metadata({}), ParentIndex(0)
		{
		}
		SymbolTableNode(std::unique_ptr<std::string> name, std::shared_ptr<SymbolMetadata> metadata, size_t depth, size_t parentIndex) : Name(std::move(name)), Metadata(std::move(metadata)), Depth(depth), ParentIndex(parentIndex)
		{
		}

		const std::string& GetName() const
		{
			return *Name.get();
		}
	};

	// a.b.c.d => a -> b -> c -> d
	class SymbolTable
	{
	private:
		std::vector<SymbolTableNode> nodes;
		StringPool stringPool;
		std::unique_ptr<Compilation> compilation = std::make_unique<Compilation>(true);
	public:
		SymbolTable()
		{
			std::unique_ptr<std::string> str = std::make_unique<std::string>("");
			nodes.push_back(SymbolTableNode(std::move(str), nullptr, 0, 0));
		}

		StringPool& GetStringPool()
		{
			return stringPool;
		}

		void Clear()
		{
			nodes.clear();
			stringPool.clear();
			nodes.emplace_back();
			compilation->Clear();
		}

		Compilation* GetCompilation() const
		{
			return compilation.get();
		}

		size_t AddNode(TextSpan name, std::shared_ptr<SymbolMetadata> metadata, size_t parentIndex)
		{
			std::unique_ptr<std::string> str = std::make_unique<std::string>(name.toString());
			size_t newIndex = nodes.size();

			nodes.push_back(SymbolTableNode(std::move(str), std::move(metadata), nodes[parentIndex].Depth + 1, parentIndex));

			auto span = TextSpan(nodes[newIndex].GetName());
			nodes[parentIndex].Children[span] = newIndex;
			return newIndex;
		}

		void SwapRemoveNode(size_t nodeIndex)
		{
			if (nodes.empty())
				return;

			size_t lastIndex = nodes.size() - 1;
			if (nodeIndex == lastIndex)
			{
				auto& removedNode = nodes[lastIndex];
				auto& parentOfRemovedNode = nodes[removedNode.ParentIndex];
				TextSpan removedSpan(removedNode.GetName());
				parentOfRemovedNode.Children.erase(removedSpan);

				nodes.pop_back();
			}
			else
			{
				std::swap(nodes[nodeIndex], nodes[lastIndex]);

				auto& movedNode = nodes[nodeIndex];
				auto& movedParent = nodes[movedNode.ParentIndex];

				for (auto& pair : movedParent.Children)
				{
					if (pair.second == lastIndex)
					{
						pair.second = nodeIndex;
						break;
					}
				}

				for (auto& pair : movedNode.Children)
				{
					nodes[pair.second].ParentIndex = nodeIndex;
				}

				auto& removedNode = nodes[lastIndex];
				auto& parentOfRemovedNode = nodes[removedNode.ParentIndex];
				TextSpan removedSpan(removedNode.GetName());
				parentOfRemovedNode.Children.erase(removedSpan);

				nodes.pop_back();
			}
		}

		void RemoveNode(size_t index)
		{
			if (index == 0 || index >= nodes.size())
				return;

			std::stack<size_t> toVisit;
			std::vector<size_t> toDelete;

			toVisit.push(index);

			while (!toVisit.empty())
			{
				size_t current = toVisit.top();
				toVisit.pop();
				toDelete.push_back(current);

				for (const auto& child : nodes[current].Children)
				{
					toVisit.push(child.second);
				}
			}

			std::reverse(toDelete.begin(), toDelete.end());

			for (size_t nodeIndex : toDelete)
			{
				SwapRemoveNode(nodeIndex);
			}
		}

		std::string GetFullyQualifiedName(size_t nodeIndex) const
		{
			std::string result;

			size_t currentNodeIndex = nodeIndex;
			while (true)
			{
				auto& node = nodes[currentNodeIndex];
				result.insert(0, node.GetName());
				currentNodeIndex = node.ParentIndex;
				if (currentNodeIndex == 0) break;
				result.insert(0, ".");
			}

			return result;
		}

		const SymbolTableNode& GetNode(size_t index) const
		{
			return nodes[index];
		}

		size_t Insert(TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t start = 0);

		size_t FindNodeIndexPart(TextSpan path, size_t startingIndex = 0) const
		{
			auto& node = nodes[startingIndex];
			auto it = node.Children.find(path);
			if (it != node.Children.end())
			{
				return it->second;
			}
			return -1;
		}

		size_t FindNodeIndexFullPath(TextSpan span, size_t startingIndex = 0) const;

		const std::shared_ptr<SymbolMetadata>& Find(TextSpan span, size_t start = 0) const
		{
			size_t index = FindNodeIndexFullPath(span, start);
			if (index == -1)
			{
				return {};
			}
			return nodes[index].Metadata;
		}

		void Strip();

		void Merge(const SymbolTable& other);

		void Write(HXSLStream& stream) const;

		void Read(HXSLStream& stream, Assembly* parentAssembly);
	};
}

#endif