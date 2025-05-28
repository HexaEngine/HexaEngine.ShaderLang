#ifndef OPERANDS_HPP
#define OPERANDS_HPP

#include "config.h"
#include "pch/std.hpp"
#include "lexical/numbers.hpp"
#include "utils/hashing.hpp"
#include "value.hpp"

namespace HXSL
{
	struct ILTypeId
	{
		uint32_t value;
		ILTypeId() = default;
		constexpr explicit ILTypeId(uint32_t val) : value(val) {}

		bool operator==(const ILTypeId& other) const { return value == other.value; }
		bool operator!=(const ILTypeId& other) const { return value != other.value; }
	};

	struct ILFieldId
	{
		uint32_t value;
		ILFieldId() = default;
		constexpr explicit ILFieldId(uint32_t val) : value(val) {}

		bool operator==(const ILFieldId& other) const { return value == other.value; }
		bool operator!=(const ILFieldId& other) const { return value != other.value; }
	};

	struct ILFuncId
	{
		uint64_t value;
		ILFuncId() = default;
		constexpr explicit ILFuncId(uint64_t val) : value(val) {}

		bool operator==(const ILFuncId& other) const { return value == other.value; }
		bool operator!=(const ILFuncId& other) const { return value != other.value; }
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
		ILTypeId typeId;
		ILFieldId fieldId;

		ILFieldAccess() = default;
		constexpr explicit ILFieldAccess(ILTypeId typeId, ILFieldId fieldId) : typeId(typeId), fieldId(fieldId) {}

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
			hash.Combine(typeId.value);
			hash.Combine(fieldId.value);
			return hash.Finalize();
		}
	};

	struct ILVarId_T
	{
		uint64_t id : 32;
		uint64_t version : 31;
		uint64_t temp : 1;
	};

	struct ILVarId
	{
		union
		{
			ILVarId_T var;
			uint64_t raw;
		};

		constexpr ILVarId(uint64_t raw) : raw(raw) {}
		constexpr ILVarId(ILVarId_T var) : var(var) {}
		constexpr ILVarId() : raw(0) {}

		constexpr operator uint64_t() const { return raw; }
		constexpr operator ILVarId_T() const { return var; }

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

		inline static bool IsPhi(const Value* op) noexcept { return op != nullptr && op->GetTypeId() == Value::PhiVal; }
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
		ILTypeId typeId;
		TypeValue(ILTypeId typeId) : Operand(ID), typeId(typeId) {}
	};

	class Function : public Operand
	{
	public:
		static constexpr Value_T ID = FuncVal;
		ILFuncId funcId;
		Function(ILFuncId funcId) : Operand(ID), funcId(funcId) {}
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

	class Phi : public Operand
	{
	public:
		static constexpr Value_T ID = PhiVal;
		ILPhiId phiId;
		Phi(ILPhiId phiId) : Operand(ID), phiId(phiId) {}
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
			hash.Combine(static_cast<const TypeValue*>(val)->typeId.value);
			break;
		case Value::FuncVal:
			hash.Combine(static_cast<const Function*>(val)->funcId.value);
			break;
		case Value::LabelVal:
			hash.Combine(static_cast<const Label*>(val)->label.value);
			break;
		case Value::FieldVal:
			hash.Combine(static_cast<const FieldAccess*>(val)->field.hash());
			break;
		case Value::PhiVal:
			hash.Combine(static_cast<const Phi*>(val)->phiId.value);
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
		case Value::PhiVal:
			return static_cast<const Phi*>(rhs)->phiId == static_cast<const Phi*>(lhs)->phiId;
		}

		return false;
	}
}

namespace std
{
	template <>
	struct hash<HXSL::ILVarId>
	{
		size_t operator()(const HXSL::ILVarId& var) const noexcept
		{
			return hash<uint64_t>{}(var.raw);
		}
	};
}

#endif