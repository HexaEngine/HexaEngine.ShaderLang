#ifndef VECTOR_H
#define VECTOR_H
#include "config.h"
#include "allocator.h"
#include <cstring>
namespace HXSL
{
	template <typename T>
	struct Vector
	{
		T* Data;
		size_t Size;
		size_t Capacity;

		Vector() : Data(nullptr), Size(0), Capacity(0) {}
		~Vector()
		{
			if (Data)
			{
				Free(Data);
				Data = nullptr;
				Capacity = 0;
				Size = 0;
			}
		}

		inline T* data() { return Data; }

		inline T* begin() { return Data; }

		inline const T* begin() const { return Data; }

		inline T* end() { return Data + Size; }

		inline const T* end() const { return Data + Size; }

		void clear() noexcept { Size = 0; }

		bool empty() const noexcept { return Size == 0; }

		size_t size() const noexcept { return Size; }

		size_t capacity() const noexcept { return Capacity; }

		void setCapacity(size_t value)
		{
			Data = ReAllocArray<T>(Data, value);
			Capacity = value;
		}

		void reserve(size_t value)
		{
			if (value > Capacity)
			{
				size_t newCapacity = Capacity * 2;
				setCapacity(value > newCapacity ? value : newCapacity);
			}
		}

		void reSize(size_t value)
		{
			reserve(value);
			Size = value;
		}

		void shrink(size_t newSize)
		{
			if (Size > newSize)
			{
				Size = newSize;
			}
		}

		void push_back(const T& value)
		{
			if (Size == Capacity)
			{
				setCapacity(Size == 0 ? 1 : Size * 2);
			}

			Data[Size] = value;
			++Size;
		}

		void pop_back()
		{
			if (Size > 0)
			{
				--Size;
			}
		}

		void remove_at(size_t index)
		{
			if (index == Size - 1)
			{
				Data[Size - 1] = { };
				Size--;
				return;
			}

			auto toMove = (Size - index - 1) * sizeof(T);
			memmove(&Data[index], &Data[index + 1], toMove);
			Size--;
		}

		void insert(size_t index, T item)
		{
			Reserve(Size + 1);

			auto toMove = (Size - index) * sizeof(T);
			memmove(&Data[index + 1], &Data[index], toMove);
			Data[index] = item;
			Size++;
		}

		bool contains(const T& v) const
		{
			const T* Data = this->Data;
			const T* Data_end = this->Data + Size;
			while (Data < Data_end)
			{
				if (*Data++ == v)
				{
					return true;
				}
			}
			return false;
		}

		T* find(const T& v)
		{
			T* Data = this->Data;
			const T* Data_end = this->Data + Size;
			while (Data < Data_end)
			{
				if (*Data == v)
				{
					break;
				}
				else
				{
					++Data;
				}
			}
			return Data;
		}

		const T* find(const T& v) const
		{
			const T* Data = this->Data;
			const T* Data_end = this->Data + Size;
			while (Data < Data_end)
			{
				if (*Data == v)
				{
					break;
				}
				else
				{
					++Data;
				}
			}

			return Data;
		}

		int find_index(const T& v) const
		{
			const T* Data_end = this->Data + Size;
			const T* it = find(v);
			if (it == Data_end)
			{
				return -1;
			}
			const ptrdiff_t off = it - this->Data;
			return (size_t)off;
		}

		T& operator[](size_t index) noexcept
		{
			return Data[index];
		}

		const T& operator[](size_t index) const noexcept
		{
			return Data[index];
		}

		const T& At(size_t index)
		{
			if (index < 0 || index >= Size)
			{
				HXSL_ASSERT(false, "Index out of bounds");
			}

			return Data[index];
		}
	};
}

#endif