#ifndef TRAILING_OBJECTS_HPP
#define TRAILING_OBJECTS_HPP

#include "pch/std.hpp"
#include "memory.hpp"

namespace HXSL
{
	namespace TrailingObjectsInternal
	{
		template <size_t Index, typename... Ts>
		struct TypeAt;

		template <typename T, typename... Ts>
		struct TypeAt<0, T, Ts...> {
			using type = T;
		};

		template <size_t Index, typename T, typename... Ts>
		struct TypeAt<Index, T, Ts...> 
		{
			static_assert(Index < sizeof...(Ts) + 1, "Index out of bounds");
			using type = typename TypeAt<Index - 1, Ts...>::type;
		};

		template <typename BaseT, typename TopTrailingObj, typename... TrailingTs>
		struct TrailingObjectsImpl;

		template <typename BaseT, typename TopTrailingObj>
		struct TrailingObjectsImpl<BaseT, TopTrailingObj>
		{
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar)
			{
				return sizeSoFar;
			}

			static constexpr size_t GetTrailingObjectsImpl(size_t offset, size_t index)
			{
				return offset;
			}
		};

		template <typename BaseT, typename TopTrailingObj, typename PrevT>
		struct TrailingObjectsImpl<BaseT, TopTrailingObj, PrevT>
		{
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar, size_t count)
			{
				return alignTo(sizeSoFar, alignof(PrevT)) + (count * sizeof(PrevT));
			}
		
			static constexpr size_t GetTrailingObjectsImpl(size_t offset, size_t index, size_t count)
			{
				offset = alignTo(offset, alignof(PrevT));
				if (index == 0) return offset;
				return offset + (count * sizeof(PrevT));
			}
		};
	
		template <typename BaseT, typename TopTrailingObj, typename PrevT, typename NextT, typename... RestT>
		struct TrailingObjectsImpl<BaseT, TopTrailingObj, PrevT, NextT, RestT...>
			: public TrailingObjectsImpl<BaseT, TopTrailingObj, NextT, RestT...>
		{
			using ParentType = TrailingObjectsImpl<BaseT, TopTrailingObj, NextT, RestT...>;

			static constexpr size_t Offset(size_t offset, size_t count)
			{
				return offset + (count * sizeof(PrevT));
			}

			template<typename... Counts>
			static constexpr size_t AdditionalSizeToAllocImpl(size_t sizeSoFar, size_t count, Counts... moreCounts)
			{
				sizeSoFar = alignTo(sizeSoFar, alignof(PrevT));
				return ParentType::AdditionalSizeToAllocImpl(Offset(sizeSoFar, count), moreCounts...);
			}

			template<typename... Counts>
			static constexpr size_t GetTrailingObjectsImpl(size_t offset, size_t index, size_t count, Counts... moreCounts)
			{
				offset = alignTo(offset, alignof(PrevT));
				if (index == 0) return offset;
				return ParentType::GetTrailingObjectsImpl(Offset(offset, count), index - 1, moreCounts...);
			}
		};
	}

	template <typename BaseT, typename... TrailingTs>
	class TrailingObjects
		: private TrailingObjectsInternal::TrailingObjectsImpl<BaseT, TrailingObjects<BaseT, TrailingTs...>, TrailingTs...>
	{	
		using TrailingObjectsImpl = TrailingObjectsInternal::TrailingObjectsImpl<BaseT, TrailingObjects<BaseT, TrailingTs...>, TrailingTs...>;

	public:
		template <size_t Index, typename... Counts>
		const auto GetTrailingObjects(Counts... counts) const
		{
			static_assert(sizeof...(Counts) == sizeof...(TrailingTs), "Counts arguments must match TrailingTs");
			using ElementType = typename TrailingObjectsInternal::TypeAt<Index, TrailingTs...>::type;
			auto offset = TrailingObjectsImpl::GetTrailingObjectsImpl(sizeof(BaseT), Index, counts...);
			return reinterpret_cast<const ElementType*>(reinterpret_cast<const char*>(this) + offset);
		}

		template <size_t Index, typename... Counts>
		auto GetTrailingObjects(Counts... counts)
		{
			static_assert(sizeof...(Counts) == sizeof...(TrailingTs), "Counts arguments must match TrailingTs");
			using ElementType = typename TrailingObjectsInternal::TypeAt<Index, TrailingTs...>::type;
			auto offset = TrailingObjectsImpl::GetTrailingObjectsImpl(sizeof(BaseT), Index, counts...);
			return reinterpret_cast<ElementType*>(reinterpret_cast<char*>(this) + offset);
		}

		template<typename... Counts>
		static constexpr size_t TotalSizeToAlloc(Counts... counts)
		{
			static_assert(sizeof...(Counts) == sizeof...(TrailingTs), "Counts arguments must match TrailingTs");
			return TrailingObjectsImpl::AdditionalSizeToAllocImpl(sizeof(BaseT), counts...);
		}

		template<typename... Counts>
		static constexpr size_t AdditionalSizeToAlloc(Counts... counts)
		{
			static_assert(sizeof...(Counts) == sizeof...(TrailingTs), "Counts arguments must match TrailingTs");
			return TotalSizeToAlloc(counts...) - sizeof(BaseT);
		}
	};
}

#endif