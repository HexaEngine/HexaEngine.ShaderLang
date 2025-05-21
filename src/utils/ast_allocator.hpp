#ifndef AST_ALLOCATOR_HPP
#define AST_ALLOCATOR_HPP

#include "pch/std.hpp"

class ASTAllocator
{
	static constexpr size_t DEFAULT_BLOCK_SIZE = 1024 * 1024;

	static constexpr size_t NUM_CLASSES = 30;
	static constexpr size_t LARGE_OBJECT_CLASS = NUM_CLASSES - 1;
	static constexpr size_t BASE_ALIGNMENT = 16;
	static constexpr size_t MAX_SIZE_CLASS = BASE_ALIGNMENT * (NUM_CLASSES - 1);

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
		size_t alignedSize = AlignUp(size, BASE_ALIGNMENT);
		return (alignedSize / BASE_ALIGNMENT) - 1;
	}

	struct AllocHeader
	{
		AllocHeader* next;
		size_t size;
	};

	struct Block
	{
		char* memory;
		size_t size;
		size_t used;

		Block(size_t size) : size(size), used(0)
		{
			memory = new char[size];
		}

		~Block()
		{
			if (memory)
			{
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

				other.memory = nullptr;
				other.size = 0;
				other.used = 0;
			}
			return *this;
		}

		Block(Block&& other) noexcept : memory(other.memory), size(other.size), used(other.used)
		{
			other.memory = nullptr;
			other.size = 0;
			other.used = 0;
		}

		void* Alloc(size_t bytes, size_t alignment)
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

			used = dataOffset + bytes;
			return memory + dataOffset;
		}
	};
public:
	std::vector<Block> blocks;
	AllocHeader* freeLists[NUM_CLASSES] = {};

	~ASTAllocator()
	{
		ReleaseAll();
	}

	void* Alloc(size_t bytes, size_t alignment = alignof(std::max_align_t))
	{
		alignment = std::max(alignment, alignof(AllocHeader));
		size_t sizeNeeded = AlignUp(bytes, alignment);
		size_t classIndex = SizeToClass(sizeNeeded);

		AllocHeader** current = &freeLists[classIndex];
		while (*current)
		{
			AllocHeader* block = *current;
			uintptr_t ptrVal = reinterpret_cast<uintptr_t>(block);
			if (block->size >= bytes && (ptrVal & (alignment - 1)) == 0)
			{
				*current = block->next;
				void* ptr = reinterpret_cast<char*>(block) + sizeof(AllocHeader);
				return ptr;
			}
			current = &block->next;
		}

		const size_t n = blocks.size();
		for (size_t i = 0; i < n; i++)
		{
			void* ptr = blocks[i].Alloc(bytes, alignment);
			if (ptr) return ptr;
		}

		size_t blockSize = AlignUp(bytes * 2, DEFAULT_BLOCK_SIZE);
		blocks.emplace_back(Block(blockSize));

		void* ptr = blocks.back().Alloc(bytes, alignment);
		return ptr;
	}

	void Free(void* ptr)
	{
		auto* header = reinterpret_cast<AllocHeader*>(static_cast<char*>(ptr) - sizeof(AllocHeader));
		size_t size = header->size;

		size_t classIndex = SizeToClass(size);

		header->next = freeLists[classIndex];
		freeLists[classIndex] = header;
	}

	void Reset()
	{
		for (size_t i = 0; i < NUM_CLASSES; i++)
		{
			freeLists[i] = nullptr;
		}

		for (auto& block : blocks)
		{
			block.used = 0;
		}
	}

	void ReleaseAll()
	{
		for (size_t i = 0; i < NUM_CLASSES; i++)
		{
			freeLists[i] = nullptr;
		}
		blocks.clear();
		blocks.shrink_to_fit();
	}
};

extern thread_local ASTAllocator astAllocator;

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

	void operator()(T* ptr) const
	{
		if (ptr)
		{
			ptr->~T();
			astAllocator.Free(base);
		}
	}
};

template <typename T>
using ast_ptr = std::unique_ptr<T, ASTAllocatorDeleter<T>>;

template <typename T, class... Args>
static ast_ptr<T> make_ast_ptr(Args&& ... args)
{
	void* rawMem = astAllocator.Alloc(sizeof(T), alignof(T));
	if (!rawMem) throw std::bad_alloc();

	T* ptr = new(rawMem) T(std::forward<Args>(args)...);
	return ast_ptr<T>(ptr, ASTAllocatorDeleter<T>{rawMem});
}

#endif