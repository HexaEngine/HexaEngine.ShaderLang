#ifndef VALUE_HPP
#define VALUE_HPP

#include "pch/std.hpp"
#include "utils/rtti_helper.hpp"

namespace HXSL
{
	namespace Backend
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

		using result_instr_checker = rtti_type_equals_checker<
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

		template <typename T>
		inline static bool isa(const Value* N) { return N && N->GetTypeId() == T::ID; }

		template<>
		inline static bool isa<ResultInstr>(const Value* N)
		{
			if (!N) return false;
			auto id = N->GetTypeId();
			return result_instr_checker::check(id);
		}

		template <typename T>
		inline static T* cast(Value* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<T*>(N); }

		template <typename T>
		inline static const T* cast(const Value* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<const T*>(N); }

		template <typename T>
		inline static T* dyn_cast(Value* N) { return isa<T>(N) ? static_cast<T*>(N) : nullptr; }

		template <typename T>
		inline static const T* dyn_cast(const Value* N) { return isa<T>(N) ? static_cast<const T*>(N) : nullptr; }
	}
}

#endif