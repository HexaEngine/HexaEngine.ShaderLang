#ifndef AST_ALLOCATOR_HPP
#define AST_ALLOCATOR_HPP

#include "pch/std.hpp"

typedef void (*DestructorPtr)(void*);

class ASTAllocator
{
	static constexpr size_t DEFAULT_BLOCK_SIZE = 1024 * 1024;

	static constexpr size_t NUM_CLASSES = 30;
	static constexpr size_t LARGE_OBJECT_CLASS = NUM_CLASSES - 1;
	static constexpr size_t OBJECT_CLASS_ALIGNMENT = 16;
	static constexpr size_t MAX_SIZE_CLASS = OBJECT_CLASS_ALIGNMENT * (NUM_CLASSES - 1);

	static size_t AlignUp(size_t size, size_t alignment)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	size_t SizeToClass(size_t size)
	{
		if (size > MAX_SIZE_CLASS)
		{
			return LARGE_OBJECT_CLASS;
		}

		return (size / OBJECT_CLASS_ALIGNMENT) - 1;
	}

	struct AllocHeader
	{
		AllocHeader* next = nullptr;
		size_t size = 0;
		DestructorPtr destructor = nullptr;
	};

	struct Block
	{
		char* memory;
		size_t size;
		AllocHeader* head;
		size_t used;

		Block(size_t size) : size(size), used(0)
		{
			memory = new char[size];
			if (size < sizeof(AllocHeader))
			{
				throw std::bad_alloc();
			}
			head = reinterpret_cast<AllocHeader*>(memory);
			new(head) AllocHeader();
			head->next = nullptr;
			head->size = 0;
			head->destructor = nullptr;
		}

		~Block()
		{
			if (memory)
			{
				AllocHeader* current = reinterpret_cast<AllocHeader*>(memory);
				while (current)
				{
					if (current->destructor)
					{
						void* objPtr = reinterpret_cast<void*>(reinterpret_cast<char*>(current) + sizeof(AllocHeader));
						current->destructor(objPtr);
					}

					current = current->next;
				}
				delete[] memory;
				memory = nullptr;
				size = 0;
				used = 0;
			}
		}

		Block(const Block&) = delete;

		Block& operator=(Block&& other) noexcept
		{
			if (this != &other)
			{
				if (memory)
				{
					delete[] memory;
				}

				memory = other.memory;
				size = other.size;
				used = other.used;
				head = reinterpret_cast<AllocHeader*>(memory);

				other.memory = nullptr;
				other.size = 0;
				other.used = 0;
			}
			return *this;
		}

		Block(Block&& other) noexcept : memory(other.memory), size(other.size), used(other.used)
		{
			head = reinterpret_cast<AllocHeader*>(memory);
			other.memory = nullptr;
			other.size = 0;
			other.used = 0;
		}

		void* Alloc(size_t bytes, size_t alignment, DestructorPtr destructor)
		{
			size_t headerOffset = AlignUp(used, alignment);
			size_t dataOffset = headerOffset + sizeof(AllocHeader);
			if (dataOffset + bytes > size)
			{
				return nullptr;
			}

			AllocHeader* header = reinterpret_cast<AllocHeader*>(memory + headerOffset);
			new(header) AllocHeader();
			header->size = bytes;
			header->destructor = destructor;

			head->next = header;
			head = header;

			used = dataOffset + bytes;
			return memory + dataOffset;
		}
	};

	struct FreeNode
	{
		AllocHeader* block;
	};

	struct FreeList
	{
		FreeNode* nodes = nullptr;
		size_t capacity = 0;
		size_t count = 0;

		~FreeList()
		{
			if (nodes)
			{
				delete[] nodes;
				nodes = nullptr;
				capacity = 0;
				count = 0;
			}
		}

		void clear()
		{
			count = 0;
		}

		void grow(size_t nextCount)
		{
			if (nextCount > capacity)
			{
				auto newCapacity = std::max(capacity * 2, nextCount);
				auto newNodes = new FreeNode[newCapacity];

				if (nodes)
				{
					std::memcpy(newNodes, nodes, capacity * sizeof(FreeNode));
					delete[] nodes;
				}

				nodes = newNodes;
				capacity = newCapacity;
			}
		}

		void push(AllocHeader* block)
		{
			size_t nextCount = count + 1;
			grow(nextCount);
			nodes[count].block = block;
			count = nextCount;
		}

		void append(FreeList& list)
		{
			auto nextCount = count + list.count;
			if (nextCount == count) return;
			grow(nextCount);
			std::memcpy(nodes + count, list.nodes, list.count * sizeof(FreeNode));
			count = nextCount;
		}

		void* pop(size_t alignment, DestructorPtr destructor)
		{
			if (count == 0)
				return nullptr;

			for (size_t i = count; i-- > 0;)
			{
				auto block = nodes[i].block;
				uintptr_t ptrVal = reinterpret_cast<uintptr_t>(block);
				if ((ptrVal & (alignment - 1)) == 0)
				{
					if (i != count - 1)
					{
						std::swap(nodes[i], nodes[count - 1]);
					}

					FreeNode* node = nodes + (count - 1);
					--count;
					auto block = node->block;
					block->destructor = destructor;

					return reinterpret_cast<char*>(block) + sizeof(AllocHeader);
				}
			}

			return nullptr;
		}
	};

public:
	struct AllocatorStats
	{
		size_t allocsPerClass[NUM_CLASSES] = {};
		size_t freePerClass[NUM_CLASSES] = {};
	};

	std::vector<Block> blocks;

	FreeList freeLists[NUM_CLASSES] = {};
	AllocatorStats stats;

	~ASTAllocator()
	{
		ReleaseAll();
	}

	const AllocatorStats& GetStats() const noexcept { return stats; }

	void* Alloc(size_t bytes, size_t alignment = alignof(std::max_align_t), DestructorPtr destructor = nullptr)
	{
		alignment = std::max(alignment, alignof(AllocHeader));
		size_t sizeNeeded = AlignUp(bytes, OBJECT_CLASS_ALIGNMENT);
		size_t classIndex = SizeToClass(sizeNeeded);

		stats.allocsPerClass[classIndex]++;

		void* freePtr = freeLists[classIndex].pop(alignment, destructor);
		if (freePtr)
		{
			stats.freePerClass[classIndex]--;
			return freePtr;
		}

		const size_t n = blocks.size();
		for (size_t i = 0; i < n; i++)
		{
			void* ptr = blocks[i].Alloc(sizeNeeded, alignment, destructor);
			if (ptr) return ptr;
		}

		size_t blockSize = AlignUp(sizeNeeded * 2, DEFAULT_BLOCK_SIZE);
		blocks.emplace_back(Block(blockSize));

		void* ptr = blocks.back().Alloc(sizeNeeded, alignment, destructor);
		return ptr;
	}

	void Free(void* ptr) noexcept
	{
		auto* header = reinterpret_cast<AllocHeader*>(static_cast<char*>(ptr) - sizeof(AllocHeader));
		if (header->destructor)
		{
			header->destructor(ptr);
		}

		size_t size = header->size;
		size_t classIndex = SizeToClass(size);
		stats.allocsPerClass[classIndex]--;
		stats.freePerClass[classIndex]++;

		header->destructor = nullptr;
		freeLists[classIndex].push(header);
	}

	void Reset() noexcept
	{
		for (size_t i = 0; i < NUM_CLASSES; i++)
		{
			freeLists[i].clear();
		}

		for (auto& block : blocks)
		{
			block.used = 0;
		}
	}

	void ReleaseAll() noexcept
	{
		for (size_t i = 0; i < NUM_CLASSES; i++)
		{
			freeLists[i].clear();
		}
		blocks.clear();
		blocks.shrink_to_fit();
	}

	void TransferAllocations(ASTAllocator* allocator)
	{
		for (auto& block : allocator->blocks)
		{
			blocks.push_back(std::move(block));
		}
		allocator->blocks.clear();

		for (size_t i = 0; i < NUM_CLASSES; i++)
		{
			FreeList& freeList = allocator->freeLists[i];
			freeLists[i].append(freeList);
			freeList.clear();
		}
	}
};

extern ASTAllocator* GetThreadAllocator();

template <typename T>
struct ASTAllocatorDeleter
{
	void* base = nullptr;

	ASTAllocatorDeleter() = default;

	template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	ASTAllocatorDeleter(const ASTAllocatorDeleter<U>& other) noexcept : base(other.base)
	{
	}

	ASTAllocatorDeleter(void* basePtr) noexcept : base(basePtr)
	{
	}

	void operator()(T* ptr) noexcept
	{
		if (base)
		{
			//GetThreadAllocator()->Free(base);
			base = nullptr;
		}
	}
};

template <typename T>
using ast_ptr = std::unique_ptr<T, ASTAllocatorDeleter<T>>;

template <typename U>
inline void destroy(void* p) { static_cast<U*>(p)->~U(); }

template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
_NODISCARD_SMART_PTR_ALLOC _CONSTEXPR23 ast_ptr<_Ty> make_ast_ptr(_Types&& ... args)
{
	void* rawMem = GetThreadAllocator()->Alloc(sizeof(_Ty), alignof(_Ty), &destroy<_Ty>);
	_Ty* ptr = new(rawMem) _Ty(std::forward<_Types>(args)...);
	return ast_ptr<_Ty>(ptr, ASTAllocatorDeleter<_Ty>{rawMem});
}

#endif