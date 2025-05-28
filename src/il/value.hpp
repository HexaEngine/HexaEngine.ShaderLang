#ifndef VALUE_HPP
#define VALUE_HPP

#include "pch/std.hpp"

namespace HXSL
{
	using ValueId = uint16_t;

	class Value
	{
	public:
		enum Value_T : uint16_t
		{
			ConstantVal,
			VariableVal,
			FieldVal,
			LabelVal,
			TypeVal,
			FuncVal,
			PhiVal,
		};

		ValueId GetTypeId() const noexcept { return typeId; }
	protected:
		constexpr Value(Value_T type) { typeId = type; }
		ValueId typeId;
	};

	template <typename T>
	inline static bool isa(const Value* N)
	{
		return N && N->GetTypeId() == T::ID;
	}

	template <typename T>
	inline static T* cast(Value* N)
	{
		assert(isa<T>(N) && "cast<T>() argument is not of type T!");
		return static_cast<T*>(N);
	}

	template <typename T>
	inline static const T* cast(const Value* N)
	{
		assert(isa<T>(N) && "cast<T>() argument is not of type T!");
		return static_cast<const T*>(N);
	}

	template <typename T>
	inline static T* dyn_cast(Value* N)
	{
		return isa<T>(N) ? static_cast<T*>(N) : nullptr;
	}

	template <typename T>
	inline static const T* dyn_cast(const Value* N)
	{
		return isa<T>(N) ? static_cast<const T*>(N) : nullptr;
	}

	template <typename T>
	inline static bool try_cast(Value* N, T*& result)
	{
		if (isa<T>(N))
		{
			result = static_cast<T*>(N);
			return true;
		}
		result = nullptr;
		return false;
	}

	template <typename T>
	inline static bool try_cast(const Value* N, const T*& result)
	{
		if (isa<T>(N))
		{
			result = static_cast<const T*>(N);
			return true;
		}
		result = nullptr;
		return false;
	}
}

#endif