#include "symbol_table.hpp"
#include "ast_modules/instantiator.hpp"

namespace HXSL
{
	SymbolType SymbolMetadata::GetSymbolType() const
	{
		return declaration->GetSymbolType();
	}

	AccessModifier SymbolMetadata::GetAccessModifiers() const
	{
		return declaration->GetAccessModifiers();
	}

	void SymbolTable::RemoveNode(SymbolTableNode* node)
	{
		std::stack<SymbolTableNode*> stack;
		stack.push(node);
		while (!stack.empty())
		{
			auto* node = stack.top();
			stack.pop();
			for (const auto& [childSpan, childNode] : node->GetChildren())
			{
				stack.push(childNode);
			}
			allocator.Free(node);
		}
	}

	SymbolHandle SymbolTable::Insert(StringSpan span, const SharedPtr<SymbolMetadata>& metadata, SymbolTableNode* start)
	{
		HXSL_ASSERT(metadata.get(), "Metadata cannot be nullptr");
		SymbolTableNode* current = start;
		if (current == nullptr)
		{
			current = root;
		}
		while (true)
		{
			size_t idx = span.indexOf('.');
			if (idx == -1) idx = span.size();
			StringSpan part = span.slice(0, idx);

			auto child = current->GetChild(part);
			if (child)
			{
				current = child;
			}
			else
			{
				current = AddNode(part, nullptr, current);
			}

			if (idx == span.size())
			{
				break;
			}
			span = span.slice(idx + 1);
		}

		if (current->GetMetadata())
		{
			return {};
		}

		current->metadata = metadata;
		return SymbolHandle(this, current);
	}

	static size_t FindSep(const StringSpan& span)
	{
		for (size_t i = 0; i < span.size(); i++)
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

	SymbolHandle SymbolTable::FindNodeIndexFullPath(StringSpan span, SymbolTableNode* startingNode) const
	{
		if (startingNode == nullptr)
		{
			return {};
		}

		auto* current = startingNode;
		while (true)
		{
			size_t idx = FindSep(span);
			if (idx == -1) idx = span.size();
			StringSpan part = span.slice(0, idx);

			auto* child = current->GetChild(part);

			if (child)
			{
				current = child;
			}
			else
			{
				return {};
			}

			if (idx == span.size())
			{
				break;
			}
			idx++;
			span = span.slice(idx);
		}

		return MakeHandle(current);
	}

	void SymbolTable::Clear()
	{
		stringPool.clear();
		RemoveNode(root);
		root = allocator.Alloc(StringSpan(), SharedPtr<SymbolMetadata>(), nullptr);
	}

	void SymbolTable::Strip()
	{
		std::stack<SymbolTableNode*> stack;
		stack.push(root);

		while (!stack.empty())
		{
			auto* node = stack.top();
			stack.pop();

			for (const auto& [childSpan, childNode] : node->GetChildren())
			{
				stack.push(childNode);
			}
		}
	}

	void SymbolMetadata::Write(Stream& stream) const
	{
		// TODO: Remove stub.
		HXSL_ASSERT(false, "SymbolMetadata::Write is not implemented.");
	}

	void SymbolMetadata::Read(Stream& stream, SymbolDef*& node, StringPool& container)
	{
		// TODO: Remove stub.
		HXSL_ASSERT(false, "SymbolMetadata::Read is not implemented.");
	}

	void SymbolTable::Write(Stream& stream) const
	{
		// TODO: Remove stub.
		HXSL_ASSERT(false, "SymbolTable::Write is not implemented.");
	}

	void SymbolTable::Read(Stream& stream, Assembly* parentAssembly)
	{
		// TODO: Remove stub.
		HXSL_ASSERT(false, "SymbolTable::Read is not implemented.");
	}

	/*
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

	void SymbolMetadata::Read(Stream& stream, SymbolDef*& node, StringPool& container)
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
			declaration = pt;
			declaration->Read(stream, container);
			node = pt;
		}
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

	static void ReadNode(Stream& stream, DiskSymbolTableNode& diskNode, std::vector<SymbolDef*>& nodes, StringPool& container, size_t& index)
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
			SymbolDef* astNode;
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

		std::vector<SymbolDef*> externalAstNodes;
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

				if (auto ns = dyn_cast<Namespace>(decl))
				{
					auto actual = externalAstNodes[idx];
					compilation->AddNamespace(dyn_cast<Namespace>(actual));
				}

				decl->Build(*this, idx, compilation, externalAstNodes);
				decl->SetAssembly(parentAssembly, MakeHandle(idx));
			}

			for (const auto& [childSpan, childIdx] : node.Children)
			{
				walkStack.push(childIdx);
			}
		}
	}*/
}