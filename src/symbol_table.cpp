#include "symbol_table.hpp"
#include <stack>

namespace HXSL
{
	void SymbolMetadata::Write(HXSLStream& stream) const
	{
		stream.WriteUInt(SymbolType);
		stream.WriteUInt(Scope);
		stream.WriteUInt(AccessModifier);
		stream.WriteUInt(Size);
		stream.WriteValue<bool>(Declaration);
		if (Declaration)
		{
			stream.WriteUInt(Declaration->GetType());
			Declaration->Write(stream);
		}
	}

	void SymbolMetadata::Read(HXSLStream& stream, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes, StringPool& container)
	{
		SymbolType = static_cast<SymbolType>(stream.ReadUInt());
		Scope = static_cast<HXSLSymbolScopeType>(stream.ReadUInt());
		AccessModifier = static_cast<HXSLAccessModifier>(stream.ReadUInt());
		Size = stream.ReadUInt();
		bool hasDeclaration = stream.ReadValue<bool>();
		if (hasDeclaration)
		{
			auto type = static_cast<HXSLNodeType>(stream.ReadUInt());
			auto pt = CreateInstance(type);
			Declaration = pt.get();
			Declaration->Read(stream, container);
			nodes.push_back(std::move(pt));
		}
		else
		{
			nodes.push_back(nullptr);
		}
	}

	size_t SymbolTable::Insert(TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t start)
	{
		size_t current = start;
		while (true)
		{
			size_t idx = span.indexOf('.');
			if (idx == -1) idx = span.Length;
			TextSpan part = span.slice(0, idx);

			auto& node = nodes[current];

			auto it = node.Children.find(part);
			if (it == node.Children.end())
			{
				current = AddNode(part, nullptr, current);
			}
			else
			{
				current = it->second;
			}

			if (idx == span.Length)
			{
				break;
			}
			span = span.slice(idx + 1);
		}

		if (nodes[current].Metadata)
		{
			return 0;
		}
		nodes[current].Metadata = metadata;
		return current;
	}

	size_t SymbolTable::FindNodeIndexFullPath(TextSpan span, size_t startingIndex) const
	{
		if (startingIndex >= nodes.size())
		{
			return -1;
		}

		size_t current = startingIndex;
		while (true)
		{
			size_t idx = span.indexOf('.');
			if (idx == -1) idx = span.Length;
			TextSpan part = span.slice(0, idx);

			size_t index = FindNodeIndexPart(part, current);

			if (index != -1)
			{
				current = index;
			}
			else
			{
				return -1;
			}

			if (idx == span.Length)
			{
				break;
			}
			idx++;
			span = span.slice(idx);
		}

		return current;
	}

	void SymbolTable::Strip()
	{
		bool found = true;
		while (found)
		{
			found = false;
			std::stack<size_t> walkStack;
			walkStack.push(0);

			while (!walkStack.empty())
			{
				size_t idx = walkStack.top();
				walkStack.pop();

				auto& node = nodes[idx];
				if (node.Metadata && node.Metadata->SymbolType == HXSLSymbolType_Scope)
				{
					RemoveNode(idx);
					found = true;
					break;
				}

				for (const auto& [childSpan, childIdx] : node.Children)
				{
					walkStack.push(childIdx);
				}
			}
		}
	}

	void SymbolTable::Merge(const SymbolTable& other)
	{
		std::stack<std::tuple<size_t, size_t>> mergeStack;
		mergeStack.push(std::make_tuple(0, 0));

		while (!mergeStack.empty())
		{
			auto [srcIndex, dstIndex] = mergeStack.top();
			mergeStack.pop();

			const SymbolTableNode& srcNode = other.nodes[srcIndex];
			for (const auto& [childSpan, srcChildIndex] : srcNode.Children)
			{
				SymbolTableNode& dstNode = nodes[dstIndex];

				const SymbolTableNode& srcChildNode = other.nodes[srcChildIndex];

				auto it = dstNode.Children.find(childSpan);
				size_t dstChildIndex;

				if (it == dstNode.Children.end())
				{
					dstChildIndex = AddNode(childSpan, srcChildNode.Metadata, dstIndex);
				}
				else
				{
					dstChildIndex = it->second;
					if (!nodes[dstChildIndex].Metadata && srcChildNode.Metadata)
					{
						nodes[dstChildIndex].Metadata = srcChildNode.Metadata;
					}
				}

				mergeStack.push(std::make_tuple(srcChildIndex, dstChildIndex));
			}
		}
	}

	static void WriteNode(HXSLStream& stream, const SymbolTableNode& node, const size_t& index)
	{
		stream.WriteUInt(index);
		stream.WriteString(node.GetName());
		stream.WriteUInt(node.Children.size());
		for (const auto& [childSpan, childIdx] : node.Children)
		{
			stream.WriteUInt(childIdx);
		}
		stream.WriteUInt(node.Depth);
		stream.WriteUInt(node.ParentIndex);

		if (node.Metadata)
		{
			stream.WriteValue<bool>(1);
			node.Metadata->Write(stream);
		}
		else
		{
			stream.WriteValue<bool>(0);
		}
	}

	void SymbolTable::Write(HXSLStream& stream) const
	{
		std::stack<size_t> walkStack;
		walkStack.push(0);

		size_t written = 0;
		size_t start = stream.Position();
		stream.WriteUInt(0);
		while (!walkStack.empty())
		{
			size_t idx = walkStack.top();
			walkStack.pop();

			auto& node = nodes[idx];
			WriteNode(stream, node, idx);
			written++;

			for (const auto& [childSpan, childIdx] : node.Children)
			{
				walkStack.push(childIdx);
			}
		}
		size_t end = stream.Position();
		stream.Position(start);
		stream.WriteUInt(written);
		stream.Position(end);
	}

	struct DiskSymbolTableNode
	{
		SymbolTableNode Node;
		std::vector<size_t> Children;
	};

	static void ReadNode(HXSLStream& stream, DiskSymbolTableNode& diskNode, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes, StringPool& container, size_t& index)
	{
		auto& node = diskNode.Node;
		index = stream.ReadUInt();
		node.Name = std::make_unique<std::string>(stream.ReadString());
		size_t children = stream.ReadUInt();
		for (size_t i = 0; i < children; i++)
		{
			diskNode.Children.push_back(stream.ReadUInt());
		}
		node.Depth = stream.ReadUInt();
		node.ParentIndex = stream.ReadUInt();
		auto metadata = stream.ReadValue<bool>();
		if (metadata)
		{
			node.Metadata = std::make_shared<SymbolMetadata>();
			node.Metadata->Read(stream, nodes, container);
		}
	}

	void SymbolTable::Read(HXSLStream& stream, Assembly* parentAssembly)
	{
		size_t nodeCount = stream.ReadUInt();

		std::vector<std::vector<size_t>> tempNodes;
		tempNodes.resize(nodeCount);
		nodes.resize(nodeCount);
		compilation->Clear();

		std::vector<std::unique_ptr<HXSLSymbolDef>> externalAstNodes;
		externalAstNodes.emplace_back();

		for (size_t i = 0; i < nodeCount; i++)
		{
			size_t index = 0;
			DiskSymbolTableNode diskNode;
			ReadNode(stream, diskNode, externalAstNodes, stringPool, index);
			nodes[index] = std::move(diskNode.Node);
			tempNodes[index] = std::move(diskNode.Children);
		}

		for (size_t i = 0; i < nodeCount; i++)
		{
			auto& node = nodes[i];
			auto& diskNode = tempNodes[i];

			for (auto idx : diskNode)
			{
				node.Children[nodes[idx].GetName()] = idx;
			}
		}

		std::stack<size_t> walkStack;
		walkStack.push(0);

		while (!walkStack.empty())
		{
			size_t idx = walkStack.top();
			walkStack.pop();

			auto& node = nodes[idx];
			if (node.Metadata && node.Metadata->SymbolType == HXSLSymbolType_Scope) continue;
			if (node.Metadata && node.Metadata->Declaration)
			{
				auto decl = node.Metadata->Declaration;

				if (auto ns = dynamic_cast<HXSLNamespace*>(decl))
				{
					auto actual = std::move(externalAstNodes[idx]).release();
					compilation->AddNamespace(std::unique_ptr<HXSLNamespace>(dynamic_cast<HXSLNamespace*>(actual)));
				}

				decl->Build(*this, idx, compilation.get(), externalAstNodes);
				decl->SetAssembly(parentAssembly, idx);
			}

			for (const auto& [childSpan, childIdx] : node.Children)
			{
				walkStack.push(childIdx);
			}
		}
	}
}