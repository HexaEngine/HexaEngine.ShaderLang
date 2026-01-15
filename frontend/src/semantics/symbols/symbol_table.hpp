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
	class SymbolMetadata : public SharedPtrBase
	{
	public:
		
		SymbolDef* declaration;
		

		SymbolMetadata(SymbolDef* declaration) : declaration(declaration)
		{
		}

		void Write(Stream& stream) const;

		void Read(Stream& stream, SymbolDef*& node, StringPool& container);


		static SymbolMetadata* Create(SymbolDef* declaration)
		{
			return new SymbolMetadata(declaration);
		}

		SymbolType GetSymbolType() const;

		AccessModifier GetAccessModifiers() const;
	};

	class SymbolTable;
	class SymbolTableNode
	{
		friend class SymbolTable;
		StringSpan name;
		dense_map<StringSpan, SymbolTableNode*> children;
		SharedPtr<SymbolMetadata> metadata; // TODO: Could get concretized since the ptr is never stored actually anywhere and we don't do merging anymore.
		SymbolTableNode* parent;

	public:
		SymbolTableNode()
		{
		}

		SymbolTableNode(const StringSpan& name, SharedPtr<SymbolMetadata>&& metadata, SymbolTableNode* parent) 
			: name(name), metadata(std::move(metadata)), parent(parent)
		{
		}

		auto& GetChildren()
		{
			return children;
		}
		 
		const auto& GetChildren() const
		{
			return children;
		}

		auto& GetMetadata()
		{
			return metadata;
		}

		auto* GetParent()
		{
			return parent;
		}

		const StringSpan& GetName() const
		{
			return name;
		}

		SymbolTableNode* GetChild(const StringSpan& path) const
		{
			auto it = children.find(path);
			if (it != children.end())
			{
				return it->second;
			}
			return nullptr;
		}
	};

	class SymbolTableNodeAllocator
	{
		static constexpr size_t PageSize = 4096;

		struct FreeNode
		{
			FreeNode* next;

			static FreeNode* CreateFrom(void* ptr, FreeNode* next = nullptr)
			{
				return new(ptr) FreeNode(next);
			}
		};

		struct Slab;
		struct SlabFooter
		{
			SymbolTableNodeAllocator* allocator;
			Slab* next;
			size_t used;

			SlabFooter(SymbolTableNodeAllocator* allocator, Slab* next) : allocator(allocator), next(next), used(0)
			{
			}
		};

		static constexpr size_t NodesPerSlab = (PageSize - sizeof(SlabFooter)) / sizeof(SymbolTableNode);
		static constexpr size_t BytesPerSlab = sizeof(SymbolTableNode) * NodesPerSlab;

		struct alignas(PageSize) Slab 
		{
			uint8_t raw[BytesPerSlab];
			SlabFooter footer;

			Slab(SymbolTableNodeAllocator* allocator, Slab* next) : footer(allocator, next)
			{
			}

			SymbolTableNode* Get(size_t index) 
			{
				return reinterpret_cast<SymbolTableNode*>(raw) + index;
			}

			SymbolTableNode* Alloc()
			{
				if (footer.used >= NodesPerSlab)
				{
					return nullptr;
				}

				auto idx = footer.used++;
				return Get(idx);
			}

			static Slab* Create(SymbolTableNodeAllocator* allocator, Slab* next)
			{
				auto* mem = aligned_alloc(alignof(Slab), sizeof(Slab));
				auto* slab = new(mem) Slab(allocator, next);
				return slab;
			}

			void Destroy()
			{
				aligned_free(this);
			}
		};

		Slab* head = nullptr;
		Slab* tail = nullptr;
		FreeNode* freeList = nullptr;
		SymbolTable* parent;

		SymbolTableNode* AllocInternal()
		{
			if (freeList)
			{
				auto p = freeList;
				freeList = p->next;
				return reinterpret_cast<SymbolTableNode*>(p);
			}

			if (auto ptr = tail->Alloc())
			{
				return ptr;
			}

			tail = Slab::Create(this, tail);
			return tail->Alloc();
		}

	public:
		SymbolTableNodeAllocator(SymbolTable* parent) : freeList(nullptr), parent(parent)
		{
			head = tail = Slab::Create(this, nullptr);
		}

		SymbolTable* GetParent()
		{
			return parent;
		}

		static SymbolTable* GetParentFromNode(SymbolTableNode* node)
		{
			auto* allocator = reinterpret_cast<SlabFooter*>(
				reinterpret_cast<uint8_t*>(node) + sizeof(SymbolTableNode) * NodesPerSlab
			)->allocator;
			return allocator->GetParent();
		}

		~SymbolTableNodeAllocator()
		{
			auto* cur = tail;
			while (cur)
			{
				auto* next = cur->footer.next;
				cur->Destroy();
				cur = next;
			}
		}

		template<typename... TArgs>
		SymbolTableNode* Alloc(TArgs&& ... args)
		{
			auto* ptr = AllocInternal();
			return new(ptr) SymbolTableNode(std::forward<TArgs>(args)...);
		}

		void Free(SymbolTableNode* node)
		{
			node->~SymbolTableNode();
			freeList = FreeNode::CreateFrom(node, freeList);
		}
	};

	// a.b.c.d => a -> b -> c -> d
	class SymbolTable
	{
	private:
		std::shared_mutex nodeMutex;
		SymbolTableNodeAllocator allocator;
		SymbolTableNode* root;
		StringPool2 stringPool;

		SymbolTableNode* AddNode(const StringSpan& name, SharedPtr<SymbolMetadata>&& metadata, SymbolTableNode* parent)
		{
			std::unique_lock<std::shared_mutex> writeLock(nodeMutex);
			auto* node = allocator.Alloc(stringPool.add(name), std::move(metadata), parent);
			parent->GetChildren()[node->GetName()] = node;
			return node;
		}

		void SwapRemoveNode(SymbolTableNode* node)
		{
			auto* parent = node->GetParent();
			parent->GetChildren().erase(node->GetName());
			allocator.Free(node);
		}

		void RemoveNode(SymbolTableNode* node);

	public:
		SymbolTable() : allocator(this)
		{
			root = allocator.Alloc(StringSpan(), SharedPtr<SymbolMetadata>(), nullptr);
		}

		~SymbolTable()
		{
			RemoveNode(root);
			root = nullptr;
		}

		StringPool2& GetStringPool()
		{
			return stringPool;
		}

		void Clear();

		bool RenameNode(const StringSpan& newName, const SymbolHandle& handle)
		{
			auto* node = handle.GetNode();
			if (node == nullptr)
			{
				return false;
			}

			StringSpan oldName = node->GetName();
			auto* parent = node->GetParent();

			auto* candidate = parent->GetChild(newName);
			if (candidate != nullptr && candidate != node)
			{
				return false;
			}

			auto str = stringPool.add(newName);

			parent->GetChildren().erase(oldName);
			node->name = str;

			parent->children[str] = node;

			return true;
		}

		std::string GetFullyQualifiedName(SymbolTableNode* node) const
		{
			std::string result;
			while (true)
			{
				result.insert(0, node->GetName());
				node = node->GetParent();
				if (node == nullptr || node->name.empty()) break;
				result.insert(0, ".");
			}

			return result;
		}

		SymbolHandle MakeHandle(SymbolTableNode* node) const
		{
			return SymbolHandle(this, node);
		}

		SymbolHandle Insert(StringSpan span, const SharedPtr<SymbolMetadata>& metadata, SymbolTableNode* start = nullptr);

		SymbolHandle FindNodeIndexPart(StringSpan path, SymbolTableNode* startingNode = nullptr) const
		{
			if (startingNode == nullptr)
			{
				startingNode = root;
			}
			auto child = startingNode->GetChild(path);
			return MakeHandle(child);
		}

		SymbolHandle FindNodeIndexFullPath(StringSpan span, SymbolTableNode* startingNode = nullptr) const;

		void Strip();

		void Write(Stream& stream) const;

		void Read(Stream& stream, Assembly* parentAssembly);
	};
}

#endif