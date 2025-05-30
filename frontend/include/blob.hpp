#ifndef HXSL_BLOB_HPP
#define HXSL_BLOB_HPP

#include "allocator.h"

namespace HXSL
{
	class Blob
	{
		void* data;
		size_t size;

	public:
		Blob(size_t capacity)
		{
			data = HXSL_Alloc(capacity);
			size = capacity;
		}

		Blob(Blob&& other) noexcept : data(other.data), size(other.size)
		{
			other.data = nullptr;
			other.size = 0;
		}

		Blob& operator=(Blob&& other) noexcept
		{
			if (this != &other)
			{
				HXSL_Free(data);
				data = other.data;
				size = other.size;
				other.data = nullptr;
				other.size = 0;
			}
			return *this;
		}

		~Blob()
		{
			if (data)
			{
				HXSL_Free(data);
				size = 0;
			}
		}

		void Clear() noexcept
		{
			if (data)
			{
				memset(data, 0, size);
			}
		}

		void Resize(size_t newSize)
		{
			auto newPtr = HXSL_ReAlloc(data, newSize);
			if (newPtr)
			{
				data = newPtr;
				size = newSize;
			}
		}

		void* GetPointer() const noexcept { return data; }

		size_t GetSize() const noexcept { return size; }
	};
}

#endif