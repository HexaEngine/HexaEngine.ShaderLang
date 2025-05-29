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
			InstructionVal,
			ResultInstrVal,
			BasicInstrVal,
			ReturnInstrVal,
			CallInstrVal,
			JumpInstrVal,
			BinaryInstrVal,
			UnaryInstrVal,
			StackAllocInstrVal,
			OffsetInstrVal,
			LoadInstrVal,
			StoreInstrVal,
			LoadParamInstrVal,
			StoreParamInstrVal,
			MoveInstrVal,
			PhiInstrVal,
			ConstantVal,
			VariableVal,
			FieldVal,
			LabelVal,
			TypeVal,
			FuncVal,
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

	template<auto... Values>
	struct value_type_equals_checker
	{
		template<typename T>
		constexpr bool operator()(T&& value) const
		{
			return ((std::forward<T>(value) == Values) || ...);
		}

		template<typename T>
		static constexpr bool check(T&& value)
		{
			return ((std::forward<T>(value) == Values) || ...);
		}
	};

	using result_instr_checker = value_type_equals_checker<
		Value::ResultInstrVal,
		Value::StackAllocInstrVal,
		Value::OffsetInstrVal,
		Value::CallInstrVal,
		Value::BinaryInstrVal,
		Value::UnaryInstrVal,
		Value::LoadInstrVal,
		Value::LoadParamInstrVal,
		Value::MoveInstrVal>;

	class ResultInstr;

	template<>
	inline static bool isa<ResultInstr>(const Value* N)
	{
		if (!N) return false;
		auto id = N->GetTypeId();
		return result_instr_checker::check(id);
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