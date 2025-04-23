#include "symbol_table.hpp"
#include <stack>

namespace HXSL
{
	void SymbolMetadata::Write(Stream& stream) const
	{
		stream.WriteUInt((uint)symbolType);
		stream.WriteUInt((uint)scope);
		stream.WriteUInt((uint)accessModifier);
		stream.WriteUInt((uint)size);
		stream.WriteValue<bool>(declaration);
		if (declaration)
		{
			stream.WriteUInt(declaration->GetType());
			declaration->Write(stream);
		}
	}

	void SymbolMetadata::Read(Stream& stream, std::unique_ptr<SymbolDef>& node, StringPool& container)
	{
		symbolType = static_cast<SymbolType>(stream.ReadUInt());
		scope = static_cast<SymbolScopeType>(stream.ReadUInt());
		accessModifier = static_cast<AccessModifier>(stream.ReadUInt());
		size = stream.ReadUInt();
		bool hasDeclaration = stream.ReadValue<bool>();
		if (hasDeclaration)
		{
			auto type = static_cast<NodeType>(stream.ReadUInt());
			auto pt = CreateInstance(type);
			declaration = pt.get();
			declaration->Read(stream, container);
			node = std::move(pt);
		}
		else
		{
			node.reset();
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
				if (node.Metadata && node.Metadata->symbolType == SymbolType_Scope)
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

	static void WriteNode(Stream& stream, const SymbolTableNode& node, const size_t& index)
	{
		stream.WriteUInt((uint)index);
		stream.WriteString(node.GetName());
		stream.WriteUInt((uint)node.Children.size());
		for (const auto& [childSpan, childIdx] : node.Children)
		{
			stream.WriteUInt((uint)childIdx);
		}
		stream.WriteUInt((uint)node.Depth);
		stream.WriteUInt((uint)node.ParentIndex);

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

	void SymbolTable::Write(Stream& stream) const
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
		stream.WriteUInt((uint)written);
		stream.Position(end);
	}

	struct DiskSymbolTableNode
	{
		SymbolTableNode Node;
		std::vector<size_t> Children;
	};

	static void ReadNode(Stream& stream, DiskSymbolTableNode& diskNode, std::vector<std::unique_ptr<SymbolDef>>& nodes, StringPool& container, size_t& index)
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
			std::unique_ptr<SymbolDef> astNode;
			node.Metadata = std::make_shared<SymbolMetadata>();
			node.Metadata->Read(stream, astNode, container);
			nodes[index] = std::move(astNode);
		}
	}

	void SymbolTable::Read(Stream& stream, Assembly* parentAssembly)
	{
		size_t nodeCount = stream.ReadUInt();

		std::vector<std::vector<size_t>> tempNodes;
		tempNodes.resize(nodeCount);
		nodes.resize(nodeCount);
		compilation->Clear();

		std::vector<std::unique_ptr<SymbolDef>> externalAstNodes;
		externalAstNodes.resize(nodeCount);

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
			if (node.Metadata && node.Metadata->symbolType == SymbolType_Scope) continue;
			if (node.Metadata && node.Metadata->declaration)
			{
				auto decl = node.Metadata->declaration;

				if (auto ns = dynamic_cast<Namespace*>(decl))
				{
					auto actual = std::move(externalAstNodes[idx]).release();
					compilation->AddNamespace(std::unique_ptr<Namespace>(dynamic_cast<Namespace*>(actual)));
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