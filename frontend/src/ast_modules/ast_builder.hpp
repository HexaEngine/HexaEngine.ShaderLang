#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include "utils/trailing_objects.hpp"
#include "ast_base.hpp"

namespace HXSL
{
	template<typename T>
	class ASTBuilder
	{
	private:
		template<typename BaseT, typename... TrailingTs>
		static std::tuple<TrailingTs...> extract_trailing_types(const TrailingObjects<BaseT, TrailingTs...>*);
		static std::tuple<> extract_trailing_types(...);

		using trailing_types = decltype(extract_trailing_types(std::declval<T*>()));

		template<typename Tuple>
		struct tuple_to_vectors;

		template<typename... Types>
		struct tuple_to_vectors<std::tuple<Types...>> { using type = std::tuple<std::vector<Types>...>; };

		using vector_storage_type = std::conditional_t<std::is_same_v<trailing_types, std::tuple<>>, std::tuple<>, typename tuple_to_vectors<trailing_types>::type>;

	public:
		static constexpr bool has_trailing_objects = !std::is_same_v<trailing_types, std::tuple<>>;
		static constexpr size_t num_trailing_types = std::tuple_size_v<trailing_types>;

		using storage_type = vector_storage_type;

	private:
		[[no_unique_address]] storage_type vectors;

	public:
		ASTBuilder() = default;

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		auto& get()
		{
			return std::get<I>(vectors);
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		const auto& get() const
		{
			return std::get<I>(vectors);
		}

		template<typename Type>
			requires has_trailing_objects
		auto& get()
		{
			return std::get<std::vector<Type>>(vectors);
		}

		template<typename Type>
			requires has_trailing_objects
		const auto& get() const
		{
			return std::get<std::vector<Type>>(vectors);
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		void push_back(const std::tuple_element_t<I, trailing_types>& item)
		{
			std::get<I>(vectors).push_back(item);
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		void push_back(std::tuple_element_t<I, trailing_types>&& item)
		{
			std::get<I>(vectors).push_back(std::move(item));
		}

		template<typename Type>
			requires has_trailing_objects
		void push_back(const Type& item)
		{
			std::get<std::vector<Type>>(vectors).push_back(item);
		}

		template<typename Type>
			requires has_trailing_objects
		void push_back(Type&& item)
		{
			std::get<std::vector<Type>>(vectors).push_back(std::move(item));
		}

		template<size_t I, typename... Args>
			requires (I < num_trailing_types&& has_trailing_objects)
		void emplace_back(Args&&... args)
		{
			std::get<I>(vectors).emplace_back(std::forward<Args>(args)...);
		}

		template<typename Type, typename... Args>
			requires has_trailing_objects
		void emplace_back(Args&&... args)
		{
			std::get<std::vector<Type>>(vectors).emplace_back(std::forward<Args>(args)...);
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		size_t size() const
		{
			return std::get<I>(vectors).size();
		}

		template<typename Type>
			requires has_trailing_objects
		size_t size() const
		{
			return std::get<std::vector<Type>>(vectors).size();
		}

		auto sizes() const
		{
			if constexpr (has_trailing_objects)
			{
				return sizes_impl(std::make_index_sequence<num_trailing_types>{});
			}
			else
			{
				return std::tuple<>{};
			}
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		void clear()
		{
			std::get<I>(vectors).clear();
		}

		template<typename Type>
			requires has_trailing_objects
		void clear()
		{
			std::get<std::vector<Type>>(vectors).clear();
		}

		void clear_all()
		{
			if constexpr (has_trailing_objects)
			{
				clear_all_impl(std::make_index_sequence<num_trailing_types>{});
			}
		}

		template<size_t I>
			requires (I < num_trailing_types&& has_trailing_objects)
		bool empty() const
		{
			return std::get<I>(vectors).empty();
		}

		template<typename Type>
			requires has_trailing_objects
		bool empty() const
		{
			return std::get<std::vector<Type>>(vectors).empty();
		}

		bool all_empty() const
		{
			if constexpr (has_trailing_objects)
			{
				return all_empty_impl(std::make_index_sequence<num_trailing_types>{});
			}
			else
			{
				return true;
			}
		}

		template<typename... Args>
			requires has_trailing_objects
		T* Finish(ASTContext* context, Args&&... args)
		{
			return FinishImpl(context, std::make_index_sequence<num_trailing_types>{}, std::forward<Args>(args)...);
		}

	private:
		template<size_t... Is>
		auto sizes_impl(std::index_sequence<Is...>) const
		{
			return std::make_tuple(std::get<Is>(vectors).size()...);
		}

		template<size_t... Is>
		void clear_all_impl(std::index_sequence<Is...>)
		{
			(std::get<Is>(vectors).clear(), ...);
		}

		template<size_t... Is>
		bool all_empty_impl(std::index_sequence<Is...>) const
		{
			return (std::get<Is>(vectors).empty() && ...);
		}

		template<size_t... Is, typename... Args>
		T* FinishImpl(ASTContext* context, std::index_sequence<Is...>, Args&&... args)
		{
			auto array_refs = std::make_tuple(get_array_ref<Is>()...);
			return T::Create(context, std::forward<Args>(args)..., std::get<Is>(array_refs)...);
		}
	};
}

#endif