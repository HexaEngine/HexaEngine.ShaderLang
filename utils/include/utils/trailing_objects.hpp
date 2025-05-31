#ifndef TRAILING_OBJECTS_HPP
#define TRAILING_OBJECTS_HPP

#include "pch/std.hpp"
#include "memory.hpp"

namespace HXSL
{
	namespace TrailingObjectsInternal
	{
		template <int Align, typename BaseT, typename TopTrailingObj, typename... TrailingTs>
		struct TrailingObjectsImpl;

		template <int Align, typename BaseT, typename TopTrailingObj>
		struct TrailingObjectsImpl<Align, BaseT, TopTrailingObj>
		{
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar)
			{
				return sizeSoFar;
			}

			template <typename T>
			const T* GetTrailingObjectsImpl(const BaseT*, T*) const
			{
				static_assert(sizeof(T) == 0, "Type not in trailing list");
				return nullptr;
			}

			template <typename T>
			T* GetTrailingObjectsImpl(BaseT*, T*)
			{
				static_assert(sizeof(T) == 0, "Type not in trailing list");
				return nullptr;
			}
		};

		template <int Align, typename BaseT, typename TopTrailingObj, typename PrevT>
		struct TrailingObjectsImpl<Align, BaseT, TopTrailingObj, PrevT>
		{
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar, size_t count)
			{
				return sizeSoFar + (count * sizeof(PrevT));
			}

			template <typename T>
			const T* GetTrailingObjectsImpl(const BaseT*, T*) const
			{
				static_assert(sizeof(T) == 0, "Type not in trailing list");
				return nullptr;
			}

			template <typename T>
			T* GetTrailingObjectsImpl(BaseT*, T*)
			{
				static_assert(sizeof(T) == 0, "Type not in trailing list");
				return nullptr;
			}

			const PrevT* GetTrailingObjectsImpl(const BaseT* Obj, PrevT*) const
			{
				auto* Ptr = reinterpret_cast<const char*>(Obj) + sizeof(BaseT);
				Ptr = reinterpret_cast<const char*>(alignTo(reinterpret_cast<uintptr_t>(Ptr), alignof(PrevT)));
				return reinterpret_cast<const PrevT*>(Ptr);
			}

			PrevT* GetTrailingObjectsImpl(BaseT* Obj, PrevT*)
			{
				auto* Ptr = reinterpret_cast<char*>(Obj) + sizeof(BaseT);
				Ptr = reinterpret_cast<char*>(alignTo(reinterpret_cast<uintptr_t>(Ptr), alignof(PrevT)));
				return reinterpret_cast<PrevT*>(Ptr);
			}
		};

		// Recursive case: two or more trailing types (PrevT, NextT, RestT...)
		template <int Align, typename BaseT, typename TopTrailingObj, typename PrevT, typename NextT, typename... RestT>
		struct TrailingObjectsImpl<Align, BaseT, TopTrailingObj, PrevT, NextT, RestT...>
			: public TrailingObjectsImpl<Align, BaseT, TopTrailingObj, NextT, RestT...>
		{
			using ParentType = TrailingObjectsImpl<Align, BaseT, TopTrailingObj, NextT, RestT...>;

			static constexpr bool RequiresRealignment()
			{
				return alignof(PrevT) < alignof(NextT);
			}

			template<typename... Counts>
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar, size_t count, Counts... moreCounts)
			{
				size_t currentSize = sizeSoFar + (count * sizeof(PrevT));
				if (RequiresRealignment())
				{
					currentSize = alignTo(currentSize, alignof(NextT));
				}
				return ParentType::AdditionalSizeToAllocImpl(currentSize, moreCounts...);
			}

			const PrevT* GetTrailingObjectsImpl(const BaseT* Obj, PrevT*) const
			{
				auto* Ptr = reinterpret_cast<const char*>(Obj) + sizeof(BaseT);
				Ptr = reinterpret_cast<const char*>(alignTo(reinterpret_cast<uintptr_t>(Ptr), alignof(PrevT)));
				return reinterpret_cast<const PrevT*>(Ptr);
			}

			PrevT* GetTrailingObjectsImpl(BaseT* Obj, PrevT*)
			{
				auto* Ptr = reinterpret_cast<char*>(Obj) + sizeof(BaseT);
				Ptr = reinterpret_cast<char*>(alignTo(reinterpret_cast<uintptr_t>(Ptr), alignof(PrevT)));
				return reinterpret_cast<PrevT*>(Ptr);
			}
		};
	}

	template <typename BaseT, typename... TrailingTs>
	class TrailingObjects
		: private TrailingObjectsInternal::TrailingObjectsImpl<1, BaseT, TrailingObjects<BaseT, TrailingTs...>, TrailingTs...>
	{
		using TrailingObjectsImpl = TrailingObjectsInternal::TrailingObjectsImpl<1, BaseT, TrailingObjects<BaseT, TrailingTs...>, TrailingTs...>;
		using TrailingObjectsImpl::GetTrailingObjectsImpl;
		template <typename T> using OverloadToken = T*;

	public:
		template <typename T>
		const T* GetTrailingObjects() const
		{
			return GetTrailingObjectsImpl(static_cast<const BaseT*>(this), OverloadToken<T>());
		}

		template <typename T>
		T* GetTrailingObjects()
		{
			return GetTrailingObjectsImpl(static_cast<BaseT*>(this), OverloadToken<T>());
		}

		template<typename... Counts>
		static constexpr size_t AdditionalSizeToAlloc(Counts... counts)
		{
			static_assert(sizeof...(Counts) == sizeof...(TrailingTs), "Counts arguments must match TrailingTs");
			return TrailingObjectsImpl::AdditionalSizeToAllocImpl(0, counts...);
		}

		template<typename... Counts>
		static constexpr size_t TotalSizeToAlloc(Counts... counts)
		{
			return sizeof(BaseT) + AdditionalSizeToAlloc(counts...);
		}
	};
}

#endif