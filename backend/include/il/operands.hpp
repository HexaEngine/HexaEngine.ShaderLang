#ifndef OPERANDS_HPP
#define OPERANDS_HPP

#include "core/config.h"
#include "pch/std.hpp"
#include "core/number.hpp"
#include "utils/hashing.hpp"
#include "value.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILTypeMetadata;

		using ILType = ILTypeMetadata*;

		class ILFuncCallMetadata;

		using ILFuncCall = ILFuncCallMetadata*;

		struct ILFieldId
		{
			uint32_t value;
			ILFieldId() = default;
			constexpr explicit ILFieldId(uint32_t val) : value(val) {}

			bool operator==(const ILFieldId& other) const { return value == other.value; }
			bool operator!=(const ILFieldId& other) const { return value != other.value; }
		};

		struct ILLabel
		{
			uint64_t value;
			ILLabel() = default;
			constexpr explicit ILLabel(uint64_t val) : value(val) {}

			bool operator==(const ILLabel& other) const { return value == other.value; }
			bool operator!=(const ILLabel& other) const { return value != other.value; }
		};

		struct ILPhiId
		{
			uint64_t value;
			ILPhiId() = default;
			constexpr explicit ILPhiId(uint64_t val) : value(val) {}

			bool operator==(const ILPhiId& other) const { return value == other.value; }
			bool operator!=(const ILPhiId& other) const { return value != other.value; }
		};

		struct ILFieldAccess
		{
			ILType typeId;
			ILFieldId fieldId;

			ILFieldAccess() = default;
			constexpr explicit ILFieldAccess(ILType typeId, ILFieldId fieldId) : typeId(typeId), fieldId(fieldId) {}

			bool operator==(const ILFieldAccess& other) const
			{
				return typeId == other.typeId && fieldId == other.fieldId;
			}

			bool operator!=(const ILFieldAccess& other) const
			{
				return !(*this == other);
			}

			uint64_t hash() const
			{
				XXHash3_64 hash{};
				hash.Combine(reinterpret_cast<size_t>(typeId));
				hash.Combine(fieldId.value);
				return hash.Finalize();
			}
		};

		struct ILVarId
		{
			static constexpr size_t IdBits = 32;
			static constexpr size_t VersionBits = 31;
			static constexpr size_t TempFlagBits = 1;

			static constexpr size_t IdShift = 0;
			static constexpr size_t VersionShift = IdShift + IdBits;
			static constexpr size_t TempFlagShift = VersionShift + VersionBits;

			static constexpr uint64_t IdMask = (static_cast<uint64_t>(1) << IdBits) - 1;
			static constexpr uint64_t VersionMask = (static_cast<uint64_t>(1) << VersionBits) - 1;
			static constexpr uint64_t TempFlagMask = (static_cast<uint64_t>(1) << TempFlagBits) - 1;
			static constexpr uint64_t TempFlagFastMask = (static_cast<uint64_t>(1) << TempFlagShift);

			static constexpr uint64_t Make(uint32_t id, uint32_t ver, bool temp)
			{
				return ((static_cast<uint64_t>(id) & IdMask) << IdShift) | ((static_cast<uint64_t>(ver) & VersionMask) << VersionShift) | (static_cast<uint64_t>(temp) << TempFlagShift);
			}

			uint64_t raw;

			constexpr ILVarId(uint64_t raw) : raw(raw) {}
			constexpr ILVarId(uint32_t id, uint32_t ver, bool temp) : raw(Make(id, ver, temp)) {}
			constexpr ILVarId() : raw(0) {}

			constexpr uint32_t id() const { return static_cast<uint32_t>(raw & IdMask); }
			constexpr uint32_t version() const { return static_cast<uint32_t>((raw >> VersionShift) & VersionMask); }
			constexpr bool temp() const { return raw & TempFlagFastMask; }

			constexpr void id(uint32_t v) { *this = WithId(v); }
			constexpr void version(uint32_t v) { *this = WithVersion(v); }
			constexpr void temp(bool v) { *this = WithTemp(v); }

			constexpr ILVarId StripId() const
			{
				return ILVarId(raw & ~(IdMask << IdShift));
			}

			constexpr ILVarId WithId(uint32_t id) const
			{
				return ILVarId(StripId().raw | static_cast<uint64_t>(id) << IdShift);
			}

			constexpr ILVarId StripVersion() const
			{
				return ILVarId(raw & ~(VersionMask << VersionShift));
			}

			constexpr ILVarId WithVersion(uint32_t version) const
			{
				return ILVarId(StripVersion().raw | static_cast<uint64_t>(version) << VersionShift);
			}

			constexpr ILVarId StripTemp() const
			{
				return ILVarId(raw & ~(TempFlagMask << TempFlagShift));
			}

			constexpr ILVarId WithTemp(bool temp) const
			{
				return ILVarId(StripTemp().raw | static_cast<uint64_t>(temp) << TempFlagShift);
			}

			constexpr ILVarId IncrementVersion()
			{
				auto prev = *this;
				*this = WithVersion(version() + 1);
				return prev;
			}

			constexpr operator uint64_t() const { return raw; }

			constexpr bool operator==(const ILVarId& other) const { return raw == other.raw; }
			constexpr bool operator!=(const ILVarId& other) const { return raw != other.raw; }

			constexpr ILVarId& operator++() { ++raw; return *this; }
			constexpr ILVarId& operator--() { --raw; return *this; }
			constexpr ILVarId operator++(int) { auto tmp = *this; ++raw; return tmp; }
			constexpr ILVarId operator--(int) { auto tmp = *this; --raw; return tmp; }
		};

		constexpr ILVarId INVALID_VARIABLE = -1;

		class Operand : public Value
		{
		protected:
			Operand(Value_T type) : Value(type) {}
		public:
			inline static bool IsDisabled(const Value* op) noexcept { return op == nullptr; }

			inline static bool IsVar(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::VariableVal; }

			inline static bool IsImm(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::ConstantVal; }

			inline static bool IsLabel(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::LabelVal; }

			inline static bool IsType(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::TypeVal; }

			inline static bool IsFunc(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::FuncVal; }

			inline static bool IsField(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::FieldVal; }
		};

		class Constant : public Operand
		{
			Number imm_m;
		public:
			static constexpr Value_T ID = ConstantVal;
			Constant(Number num) : Operand(ID), imm_m(num) {}

			Number& imm() { return imm_m; }

			const Number& imm() const { return imm_m; }
		};

		class Variable : public Operand
		{
		public:
			static constexpr Value_T ID = VariableVal;
			ILVarId varId;
			Variable(ILVarId varId) : Operand(ID), varId(varId) {}

			operator ILVarId() const { return varId; }
		};

		class TypeValue : public Operand
		{
		public:
			static constexpr Value_T ID = TypeVal;
			ILType typeId;
			TypeValue(ILType typeId) : Operand(ID), typeId(typeId) {}
		};

		class Function : public Operand
		{
		public:
			static constexpr Value_T ID = FuncVal;
			ILFuncCall funcId;
			Function(ILFuncCall funcId) : Operand(ID), funcId(funcId) {}
		};

		class Label : public Operand
		{
		public:
			static constexpr Value_T ID = LabelVal;
			ILLabel label;
			Label(ILLabel label) : Operand(ID), label(label) {}
		};

		class FieldAccess : public Operand
		{
		public:
			static constexpr Value_T ID = FieldVal;
			ILFieldAccess field;
			FieldAccess(ILFieldAccess field) : Operand(ID), field(field) {}
		};

		static uint64_t hash(const Value* val) noexcept
		{
			if (!val) return 0;
			XXHash3_64 hash{};
			hash.Combine(static_cast<uint32_t>(val->GetTypeId()));

			switch (val->GetTypeId())
			{
			case Value::ConstantVal:
				hash.Combine(static_cast<const Constant*>(val)->imm().hash());
				break;
			case Value::VariableVal:
				hash.Combine(static_cast<const Variable*>(val)->varId.raw);
				break;
			case Value::TypeVal:
				hash.Combine(reinterpret_cast<size_t>(static_cast<const TypeValue*>(val)->typeId));
				break;
			case Value::FuncVal:
				hash.Combine(reinterpret_cast<size_t>(static_cast<const Function*>(val)->funcId));
				break;
			case Value::LabelVal:
				hash.Combine(static_cast<const Label*>(val)->label.value);
				break;
			case Value::FieldVal:
				hash.Combine(static_cast<const FieldAccess*>(val)->field.hash());
				break;
			}
			return hash.Finalize();
		}

		static bool equals(const Value* rhs, const Value* lhs)
		{
			if (rhs == nullptr || lhs == nullptr) return rhs == lhs;
			auto typeId = rhs->GetTypeId();
			if (typeId != lhs->GetTypeId()) return false;

			switch (typeId)
			{
			case Value::ConstantVal:
				return static_cast<const Constant*>(rhs)->imm() == static_cast<const Constant*>(lhs)->imm();
			case Value::VariableVal:
				return static_cast<const Variable*>(rhs)->varId == static_cast<const Variable*>(lhs)->varId;
			case Value::TypeVal:
				return static_cast<const TypeValue*>(rhs)->typeId == static_cast<const TypeValue*>(lhs)->typeId;
			case Value::FuncVal:
				return static_cast<const Function*>(rhs)->funcId == static_cast<const Function*>(lhs)->funcId;
			case Value::LabelVal:
				return static_cast<const Label*>(rhs)->label == static_cast<const Label*>(lhs)->label;
			case Value::FieldVal:
				return static_cast<const FieldAccess*>(rhs)->field == static_cast<const FieldAccess*>(lhs)->field;
			}

			return false;
		}
	}
}

namespace std
{
	template <>
	struct hash<HXSL::Backend::ILVarId>
	{
		size_t operator()(const HXSL::Backend::ILVarId& var) const noexcept
		{
			return hash<uint64_t>{}(var.raw);
		}
	};

	template <>
	struct hash<HXSL::Backend::ILLabel>
	{
		size_t operator()(const HXSL::Backend::ILLabel& var) const noexcept
		{
			return hash<uint64_t>{}(var.value);
		}
	};
}

#endif