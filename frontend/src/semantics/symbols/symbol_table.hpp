#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "pch/ast.hpp"
#include "symbol_handle.hpp"

#include "io/stream.hpp"
#include "utils/string_pool.hpp"
#include "utils/span.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	class SymbolMetadata
	{
	public:
		std::atomic<size_t> refCount{ 1 };
		SymbolDef* declaration;

		SymbolMetadata(SymbolDef* declaration) : declaration(declaration)
		{
		}

		void Write(Stream& stream) const;

		void Read(Stream& stream, SymbolDef*& node, StringPool& container);

		size_t AddRef()
		{
			return refCount.fetch_add(1, std::memory_order_acq_rel) + 1;
		}

		void Release()
		{
			if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				delete this;
			}
		}
	};

	template <typename T>
	struct SharedPtr
	{
		T* ptr;

		SharedPtr() : ptr(nullptr)
		{
		}

		SharedPtr(T* p) : ptr(p)
		{
			if (ptr)
			{
				ptr->AddRef();
			}
		}

		SharedPtr(const SharedPtr<T>& other) : ptr(other.ptr)
		{
			if (ptr)
			{
				ptr->AddRef();
			}
		}

		SharedPtr<T>& operator=(const SharedPtr<T>& other)
		{
			if (ptr != other.ptr)
			{
				if (ptr)
				{
					ptr->Release();
				}
				ptr = other.ptr;
				if (ptr)
				{
					ptr->AddRef();
				}
			}
			return *this;
		}

		~SharedPtr()
		{
			if (ptr)
			{
				ptr->Release();
			}
		}

		T* operator->() const
		{
			return ptr;
		}

		T& operator*() const
		{
			return *ptr;
		}

		operator bool() const
		{
			return ptr != nullptr;
		}

		void reset()
		{
			if (ptr)
			{
				ptr->Release();
				ptr = nullptr;
			}
		}

		T* get() const
		{
			return ptr;
		}

		T* detach()
		{
			T* temp = ptr;
			ptr = nullptr;
			return temp;
		}

		static SharedPtr<T> make_shared(T* p)
		{
			return SharedPtr<T>(p);
		}
	};

	struct SymbolTableNode
	{
		std::unique_ptr<std::string> Name;
		dense_map<StringSpan, size_t> Children;
		SharedPtr<SymbolMetadata> Metadata; // TODO: Could get concretized since the ptr is never stored actually anywhere and we don't do merging anymore.
		std::shared_ptr<size_t> handle; // Pointer to own index, this is crucial to support stable indices even if the node gets moved see SymbolHandle for more info.
		size_t Depth;
		size_t ParentIndex;

		SymbolTableNode() : Depth(0), Metadata({}), ParentIndex(0)
		{
		}

		SymbolTableNode(std::unique_ptr<std::string> name, SharedPtr<SymbolMetadata> metadata, std::shared_ptr<size_t> handle, size_t depth, size_t parentIndex) : Name(std::move(name)), Metadata(std::move(metadata)), handle(std::move(handle)), Depth(depth), ParentIndex(parentIndex)
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
		static constexpr size_t rootIndex = 0;
		std::atomic<size_t> counter{ 0 };
		size_t capacity = 1024;
		std::vector<SymbolTableNode> nodes;
		std::shared_mutex nodeMutex;
		StringPool stringPool;

		void RemoveRange(const size_t& start, const size_t& end);

		size_t AddNode(StringSpan name, std::shared_ptr<SymbolMetadata> metadata, size_t parentIndex)
		{
			size_t index = counter.fetch_add(1, std::memory_order_relaxed);

			if (index >= capacity)
			{
				std::unique_lock<std::shared_mutex> writeLock(nodeMutex);
				if (index >= capacity)
				{
					capacity *= 2;
					nodes.resize(capacity);
				}
			}

			std::shared_lock<std::shared_mutex> readLock(nodeMutex);

			std::unique_ptr<std::string> str = std::make_unique<std::string>(name.str());
			nodes[index] = SymbolTableNode(std::move(str), std::move(metadata), std::make_shared<size_t>(index), nodes[parentIndex].Depth + 1, parentIndex);

			nodes[parentIndex].Children[nodes[index].GetName()] = index;

			return index;
		}

		size_t FindNodeIndexPartInternal(StringSpan path, size_t startingIndex = 0) const
		{
			auto& node = nodes[startingIndex];
			auto it = node.Children.find(path);
			if (it != node.Children.end())
			{
				return it->second;
			}
			return -1;
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
				parentOfRemovedNode.Children.erase(removedNode.GetName());

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
				parentOfRemovedNode.Children.erase(removedNode.GetName());

				nodes.pop_back();
			}
		}

	public:
		SymbolTable()
		{
			nodes.resize(capacity);
			std::unique_ptr<std::string> str = std::make_unique<std::string>("");
			nodes[0] = (SymbolTableNode(std::move(str), nullptr, std::make_shared<size_t>(0), 0, 0));
			counter.store(1);
		}

		StringPool& GetStringPool()
		{
			return stringPool;
		}

		void Clear();

		void ShrinkToFit()
		{
			capacity = counter.load();
			nodes.resize(capacity);
		}

		bool RenameNode(const StringSpan& newName, const SymbolHandle& handle)
		{
			auto index = handle.GetIndex();
			if (index == 0 || index >= nodes.size())
			{
				return false;
			}

			auto& node = nodes[index];
			StringSpan oldName = *node.Name.get();
			auto& parent = nodes[node.ParentIndex];

			auto it = parent.Children.find(newName);
			if (it != parent.Children.end() && it->second != index)
			{
				return false;
			}

			std::unique_ptr<std::string> str = std::make_unique<std::string>(newName.str());

			parent.Children.erase(oldName);
			node.Name = std::move(str);

			parent.Children[node.GetName()] = index;

			return true;
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

		SymbolHandle MakeHandle(size_t index) const
		{
			return SymbolHandle(this, nodes[index].handle);
		}

		SymbolHandle Insert(StringSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t start = 0);

		SymbolHandle FindNodeIndexPart(StringSpan path, size_t startingIndex = 0) const
		{
			auto index = FindNodeIndexPartInternal(path, startingIndex);
			if (index != SymbolHandle::Invalid)
			{
				return MakeHandle(index);
			}
			return {};
		}

		SymbolHandle FindNodeIndexFullPath(StringSpan span, size_t startingIndex = 0) const;

		void Strip();

		/// <summary>
		/// Sorts the tree in a DFS fasion, meant for improving cache locality, if you inserted nodes in a random order.
		/// </summary>
		void Sort();

		void Write(Stream& stream) const;

		void Read(Stream& stream, Assembly* parentAssembly);
	};
}

#endif