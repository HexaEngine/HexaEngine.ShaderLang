#include "symbol_table.hpp"
#include "ast_modules/instantiator.hpp"

namespace HXSL
{
	void SymbolMetadata::Write(Stream& stream) const
	{
		stream.WriteUInt((uint32_t)symbolType);
		stream.WriteUInt((uint32_t)scope);
		stream.WriteUInt((uint32_t)accessModifier);
		stream.WriteUInt((uint32_t)size);
		if (symbolType == SymbolType_Namespace)
		{
			stream.WriteValue<bool>(false);
			return;
		}
		stream.WriteValue<bool>(declaration);
		if (declaration)
		{
			stream.WriteUInt(declaration->GetType());
			declaration->Write(stream);
		}
	}

	void SymbolMetadata::Read(Stream& stream, ast_ptr<SymbolDef>& node, StringPool& container)
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

	SymbolHandle SymbolTable::Insert(StringSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t start)
	{
		HXSL_ASSERT(metadata.get(), "Metadata cannot be nullptr");
		size_t current = start;
		while (true)
		{
			size_t idx = span.indexOf('.');
			if (idx == -1) idx = span.length;
			StringSpan part = span.slice(0, idx);

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

			if (idx == span.length)
			{
				break;
			}
			span = span.slice(idx + 1);
		}

		auto& node = nodes[current];
		if (node.Metadata)
		{
			return {};
		}

		node.Metadata = metadata;
		return SymbolHandle(this, node.handle);
	}

	static size_t FindSep(const StringSpan& span)
	{
		for (size_t i = 0; i < span.length; i++)
		{
			const char& c = span[i];
			if (c == QUALIFIER_SEP)
			{
				return i;
			}
			else if (c == '(')
			{
				return -1;
			}
		}
		return -1;
	}

	SymbolHandle SymbolTable::FindNodeIndexFullPath(StringSpan span, size_t startingIndex) const
	{
		if (startingIndex >= nodes.size())
		{
			return {};
		}

		size_t current = startingIndex;
		while (true)
		{
			size_t idx = FindSep(span);
			if (idx == -1) idx = span.length;
			StringSpan part = span.slice(0, idx);

			size_t index = FindNodeIndexPartInternal(part, current);

			if (index != -1)
			{
				current = index;
			}
			else
			{
				return {};
			}

			if (idx == span.length)
			{
				break;
			}
			idx++;
			span = span.slice(idx);
		}

		return MakeHandle(current);
	}

	void SymbolTable::RemoveRange(const size_t& start, const size_t& end)
	{
		const size_t size = nodes.size();
		size_t delta = (end - start);
		nodes.erase(nodes.begin() + start, nodes.begin() + end);

		if (end != size)
		{
			for (size_t i = start; i < nodes.size(); i++)
			{
				auto& node = nodes[i];
				if (node.ParentIndex > start)
				{
					node.ParentIndex -= delta;
				}

				for (auto& pair : node.Children)
				{
					if (pair.second > start)
					{
						pair.second -= delta;
					}
				}
			}
		}
	}

	void SymbolTable::Clear()
	{
		nodes.clear();
		stringPool.clear();
		nodes.emplace_back();
		compilation->Clear();
	}

	void SymbolTable::Strip()
	{
		std::vector<size_t> toRemove;
		for (size_t i = 0; i < nodes.size(); i++)
		{
			auto& node = nodes[i];
			auto& type = node.Metadata->symbolType;
			if (type == SymbolType_Scope || type == SymbolType_Variable)
			{
				toRemove.push_back(i);
				auto& parent = nodes[node.ParentIndex];
				parent.Children.erase(*node.Name.get());
				node.ParentIndex = 0;
			}
		}

		if (toRemove.empty())
		{
			return;
		}

		std::sort(toRemove.begin(), toRemove.end(), std::greater<size_t>());

		size_t last = toRemove[0];
		size_t start = last;

		const size_t size = toRemove.size();
		for (size_t i = 1; i < size; i++)
		{
			auto& index = toRemove[i];
			if (last == index + 1)
			{
				last = index;
				continue;
			}
			else
			{
				RemoveRange(start, last + 1);
				start = index;
			}
		}

		RemoveRange(start, last + 1);
	}

	void SymbolTable::Sort()
	{
		std::vector<SymbolTableNode> output;
		output.reserve(nodes.size());

		std::stack<std::unordered_map<StringSpan, size_t>::iterator> walkStack;

		output.push_back(std::move(nodes[rootIndex]));
		auto& rootNode = output[rootIndex];

		for (auto it = rootNode.Children.begin(); it != rootNode.Children.end(); ++it)
		{
			walkStack.push(it);
		}

		while (!walkStack.empty())
		{
			auto p = walkStack.top();
			walkStack.pop();

			auto newIndex = output.size();
			output.push_back(std::move(nodes[p->second]));

			auto& node = output[newIndex];
			*node.handle = newIndex;
			p->second = newIndex;

			for (auto it = node.Children.begin(); it != node.Children.end(); ++it)
			{
				nodes[it->second].ParentIndex = newIndex;
				walkStack.push(it);
			}
		}

		nodes = std::move(output);
	}

	static void WriteNode(Stream& stream, const SymbolTableNode& node, const size_t& index)
	{
		stream.WriteUInt((uint32_t)index);
		stream.WriteString(node.GetName());
		stream.WriteUInt((uint32_t)node.Children.size());
		for (const auto& [childSpan, childIdx] : node.Children)
		{
			stream.WriteUInt((uint32_t)childIdx);
		}
		stream.WriteUInt((uint32_t)node.Depth);
		stream.WriteUInt((uint32_t)node.ParentIndex);

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
		stream.WriteUInt((uint32_t)written);
		stream.Position(end);
	}

	struct DiskSymbolTableNode
	{
		SymbolTableNode Node;
		std::vector<size_t> Children;
	};

	static void ReadNode(Stream& stream, DiskSymbolTableNode& diskNode, std::vector<ast_ptr<SymbolDef>>& nodes, StringPool& container, size_t& index)
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
			ast_ptr<SymbolDef> astNode;
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

		std::vector<ast_ptr<SymbolDef>> externalAstNodes;
		externalAstNodes.resize(nodeCount);

		for (size_t i = 0; i < nodeCount; i++)
		{
			size_t index = 0;
			DiskSymbolTableNode diskNode;
			ReadNode(stream, diskNode, externalAstNodes, stringPool, index);
			nodes[index] = std::move(diskNode.Node);
			nodes[index].handle = std::make_shared<size_t>(index);
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
					compilation->AddNamespace(ast_ptr<Namespace>(dynamic_cast<Namespace*>(actual)));
				}

				decl->Build(*this, idx, compilation.get(), externalAstNodes);
				decl->SetAssembly(parentAssembly, MakeHandle(idx));
			}

			for (const auto& [childSpan, childIdx] : node.Children)
			{
				walkStack.push(childIdx);
			}
		}
	}
}