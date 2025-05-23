#ifndef NUMBERS_HPP
#define NUMBERS_HPP

#include "utils/text_helper.hpp"
#include "pch/std.hpp"
#include "utils/hashing.hpp"
#include <utils/half.hpp>

namespace HXSL
{
	using half_float::half;

	enum NumberType : char
	{
		NumberType_Unknown,
		NumberType_Int8,
		NumberType_UInt8,
		NumberType_Int16,
		NumberType_UInt16,
		NumberType_Int32,
		NumberType_UInt32,
		NumberType_Int64,
		NumberType_UInt64,
		NumberType_Half,
		NumberType_Float,
		NumberType_Double,
	};

	struct Number
	{
		constexpr Number() : Kind(NumberType_Unknown), u64(0)
		{
		}

		Number(uint8_t value) : Kind(NumberType_UInt8), u8(value)
		{
		}

		Number(int8_t value) : Kind(NumberType_Int8), i8(value)
		{
		}

		Number(uint16_t value) : Kind(NumberType_UInt16), u16(value)
		{
		}

		Number(int16_t value) : Kind(NumberType_Int16), i16(value)
		{
		}

		Number(uint32_t value) : Kind(NumberType_UInt32), u32(value)
		{
		}

		Number(int32_t value) : Kind(NumberType_Int32), i32(value)
		{
		}

		Number(uint64_t value) : Kind(NumberType_UInt64), u64(value)
		{
		}

		Number(int64_t value) : Kind(NumberType_Int64), i64(value)
		{
		}

		Number(half value) : Kind(NumberType_Half), half_(value)
		{
		}

		Number(float value) : Kind(NumberType_Float), float_(value)
		{
		}

		Number(double value) : Kind(NumberType_Double), double_(value)
		{
		}

		Number(bool value) : Kind(NumberType_UInt8), u8(value)
		{
		}

		NumberType Kind;
		union {
			int8_t i8;
			uint8_t u8;
			int16_t i16;
			uint16_t u16;
			int32_t i32;
			uint32_t u32;
			int64_t i64;
			uint64_t u64;
			half half_;
			float float_;
			double double_;
		};

		static Number implicitCast(const Number& n, NumberType targetType)
		{
			if (n.Kind == targetType)
			{
				return n;
			}

			Number result;
			result.Kind = targetType;

			switch (n.Kind)
			{
			case NumberType_Int8:
				if (targetType == NumberType_Int16) result.i16 = n.i8;
				else if (targetType == NumberType_Int32) result.i32 = n.i8;
				else if (targetType == NumberType_Int64) result.i64 = n.i8;
				else if (targetType == NumberType_Half) result.half_ = n.i8;
				else if (targetType == NumberType_Float) result.float_ = n.i8;
				else if (targetType == NumberType_Double) result.double_ = n.i8;
				break;

			case NumberType_UInt8:
				if (targetType == NumberType_UInt16) result.u16 = n.u8;
				else if (targetType == NumberType_UInt32) result.u32 = n.u8;
				else if (targetType == NumberType_UInt64) result.u64 = n.u8;
				else if (targetType == NumberType_Int16) result.i16 = n.u8;
				else if (targetType == NumberType_Int32) result.i32 = n.u8;
				else if (targetType == NumberType_Int64) result.i64 = n.u8;
				else if (targetType == NumberType_Half) result.half_ = n.u8;
				else if (targetType == NumberType_Float) result.float_ = n.u8;
				else if (targetType == NumberType_Double) result.double_ = n.u8;
				break;

			case NumberType_Int16:
				if (targetType == NumberType_Int32) result.i32 = n.i16;
				else if (targetType == NumberType_Int64) result.i64 = n.i16;
				else if (targetType == NumberType_Half) result.half_ = n.i16;
				else if (targetType == NumberType_Float) result.float_ = n.i16;
				else if (targetType == NumberType_Double) result.double_ = n.i16;
				break;

			case NumberType_UInt16:
				if (targetType == NumberType_UInt32) result.u32 = n.u16;
				else if (targetType == NumberType_UInt64) result.u64 = n.u16;
				else if (targetType == NumberType_Int32) result.i32 = n.u16;
				else if (targetType == NumberType_Int64) result.i64 = n.u16;
				else if (targetType == NumberType_Half) result.half_ = n.u16;
				else if (targetType == NumberType_Float) result.float_ = n.u16;
				else if (targetType == NumberType_Double) result.double_ = n.u16;
				break;

			case NumberType_Int32:
				if (targetType == NumberType_Int64) result.i64 = n.i32;
				else if (targetType == NumberType_Float) result.float_ = static_cast<float>(n.i32);
				else if (targetType == NumberType_Double) result.double_ = static_cast<double>(n.i32);
				break;

			case NumberType_UInt32:
				if (targetType == NumberType_UInt64) result.u64 = n.u32;
				else if (targetType == NumberType_Int64) result.i64 = n.i32;
				else if (targetType == NumberType_Float) result.float_ = static_cast<float>(n.u32);
				else if (targetType == NumberType_Double) result.double_ = static_cast<double>(n.u32);
				break;

			case NumberType_UInt64:
				if (targetType == NumberType_Double) result.double_ = static_cast<double>(n.u64);
				break;

			case NumberType_Int64:
				if (targetType == NumberType_Double) result.double_ = static_cast<double>(n.i64);
				break;

			case NumberType_Half:
				if (targetType == NumberType_Float) result.float_ = n.half_;
				else if (targetType == NumberType_Double) result.double_ = n.half_;
				break;

			case NumberType_Float:
				if (targetType == NumberType_Double) result.double_ = n.float_;
				break;

			default:
				result.Kind = NumberType_Unknown;
				break;
			}

			return result;
		}

#define MAKE_IMPL(name,op) \
		static Number name##Impl(Number rhl, Number lhl) { \
			if (rhl.Kind != lhl.Kind) { if (rhl.Kind > lhl.Kind) lhl = implicitCast(lhl, rhl.Kind); else rhl = implicitCast(rhl, lhl.Kind); } \
			Number num; num.Kind = rhl.Kind; \
			switch (num.Kind) { \
			case NumberType_Unknown: return {}; \
			case NumberType_Int8: num.i8 = rhl.i8 op lhl.i8; return num; \
			case NumberType_UInt8: num.u8 = rhl.u8 op lhl.u8; return num; \
			case NumberType_Int16: num.i16 = rhl.i16 op lhl.i16; return num; \
			case NumberType_UInt16: num.u16 = rhl.u16 op lhl.u16; return num; \
			case NumberType_Int32: num.i32 = rhl.i32 op lhl.i32; return num; \
			case NumberType_UInt32: num.u32 = rhl.u32 op lhl.u32; return num; \
			case NumberType_Int64: num.i64 = rhl.i64 op lhl.i64; return num; \
			case NumberType_UInt64: num.u64 = rhl.u64 op lhl.u64; return num; \
			case NumberType_Half: num.half_ = rhl.half_ op lhl.half_; return num; \
			case NumberType_Float: num.float_ = rhl.float_ op lhl.float_; return num; \
			case NumberType_Double: num.double_ = rhl.double_ op lhl.double_; return num; \
			} \
			num = {}; return num;  \
		} Number operator op(const Number& other) const { return name##Impl(*this, other); }

#define MAKE_IMPL_UNARY(name,op) \
		static Number name##Impl(Number rhl) { \
			Number num; num.Kind = rhl.Kind; \
			switch (num.Kind) { \
			case NumberType_Unknown: return {}; \
			case NumberType_Int8: num.i8 = op##rhl.i8; return num; \
			case NumberType_Int16: num.i16 = op##rhl.i16; return num; \
			case NumberType_Int32: num.i32 = op##rhl.i32; return num; \
			case NumberType_Int64: num.i64 = op##rhl.i64; return num; \
			case NumberType_Half: num.half_ = op##rhl.half_; return num; \
			case NumberType_Float: num.float_ = op##rhl.float_; return num; \
			case NumberType_Double: num.double_ = op##rhl.double_; return num; \
			} \
			num = {}; return num;  \
		} Number operator op() const { return name##Impl(*this); }

#define MAKE_IMPL_INT(name,op) \
		static Number name##Impl(Number rhl, Number lhl) { \
			if (rhl.Kind != lhl.Kind) { if (rhl.Kind > lhl.Kind) lhl = implicitCast(lhl, rhl.Kind); else rhl = implicitCast(rhl, lhl.Kind); } \
			Number num; num.Kind = rhl.Kind; \
			switch (num.Kind) { \
			case NumberType_Unknown: return {}; \
			case NumberType_Int8: num.i8 = rhl.i8 op lhl.i8; return num; \
			case NumberType_UInt8: num.u8 = rhl.u8 op lhl.u8; return num; \
			case NumberType_Int16: num.i16 = rhl.i16 op lhl.i16; return num; \
			case NumberType_UInt16: num.u16 = rhl.u16 op lhl.u16; return num; \
			case NumberType_Int32: num.i32 = rhl.i32 op lhl.i32; return num; \
			case NumberType_UInt32: num.u32 = rhl.u32 op lhl.u32; return num; \
			case NumberType_Int64: num.i64 = rhl.i64 op lhl.i64; return num; \
			case NumberType_UInt64: num.u64 = rhl.u64 op lhl.u64; return num; \
			} \
			num = {}; return num;  \
		} Number operator op(const Number& other) const { return name##Impl(*this, other); }

#define MAKE_IMPL_INT_UNARY(name,op) \
		static Number name##Impl(Number rhl) { \
			Number num; num.Kind = rhl.Kind; \
			switch (num.Kind) { \
			case NumberType_Unknown: return {}; \
			case NumberType_Int8: num.i8 = op##rhl.i8; return num; \
			case NumberType_UInt8: num.u8 = op##rhl.u8; return num; \
			case NumberType_Int16: num.i16 = op##rhl.i16; return num; \
			case NumberType_UInt16: num.u16 = op##rhl.u16; return num; \
			case NumberType_Int32: num.i32 = op##rhl.i32; return num; \
			case NumberType_UInt32: num.u32 = op##rhl.u32; return num; \
			case NumberType_Int64: num.i64 = op##rhl.i64; return num; \
			case NumberType_UInt64: num.u64 = op##rhl.u64; return num; \
			} \
			num = {}; return num; \
		} Number operator op() const { return name##Impl(*this); }

#define MAKE_IMPL_CMP(name,op) \
		static int name##Impl(Number rhl, Number lhl) { \
			if (rhl.Kind != lhl.Kind) { if (rhl.Kind > lhl.Kind) lhl = implicitCast(lhl, rhl.Kind); else rhl = implicitCast(rhl, lhl.Kind); } \
			switch (rhl.Kind) { \
			case NumberType_Unknown: return -1; \
			case NumberType_Int8: return rhl.i8 op lhl.i8; \
			case NumberType_UInt8: return rhl.u8 op lhl.u8; \
			case NumberType_Int16: return rhl.i16 op lhl.i16; \
			case NumberType_UInt16: return rhl.u16 op lhl.u16; \
			case NumberType_Int32: return rhl.i32 op lhl.i32; \
			case NumberType_UInt32: return rhl.u32 op lhl.u32; \
			case NumberType_Int64: return rhl.i64 op lhl.i64; \
			case NumberType_UInt64: return rhl.u64 op lhl.u64; \
			case NumberType_Half: return rhl.half_ op lhl.half_; \
			case NumberType_Float: return rhl.float_ op lhl.float_; \
			case NumberType_Double: return rhl.double_ op lhl.double_; \
			} \
			return -1; \
		} int operator op(const Number& other) const { return name##Impl(*this, other); }

		MAKE_IMPL(ADD, +);

		MAKE_IMPL(SUB, -);

		MAKE_IMPL(MUL, *);

		MAKE_IMPL(DIV, / );

		MAKE_IMPL_INT(MOD, %);

		MAKE_IMPL_INT(SL, << );

		MAKE_IMPL_INT(SR, >> );

		MAKE_IMPL_INT(AND, &);

		MAKE_IMPL_INT(OR, | );

		MAKE_IMPL_INT(XOR, ^);

		MAKE_IMPL_CMP(EQ, == );

		MAKE_IMPL_CMP(NEQ, != );

		MAKE_IMPL_CMP(GEQ, >= );

		MAKE_IMPL_CMP(LEQ, <= );

		MAKE_IMPL_CMP(GE, > );

		MAKE_IMPL_CMP(LE, < );

		MAKE_IMPL_INT_UNARY(NOT, ~);

		MAKE_IMPL_UNARY(NEG, -);

		bool ToBool() const
		{
			switch (Kind)
			{
			case NumberType_Int8: return i8 != 0;
			case NumberType_UInt8: return u8 != 0;
			case NumberType_Int16: return i16 != 0;
			case NumberType_UInt16: return u16 != 0;
			case NumberType_Int32: return i32 != 0;
			case NumberType_UInt32: return u32 != 0;
			case NumberType_Int64: return i64 != 0;
			case NumberType_UInt64: return u64 != 0;
			case NumberType_Half: return half_ != 0.0f;
			case NumberType_Float: return float_ != 0.0f;
			case NumberType_Double: return double_ != 0.0;
			default: return false;
			}
		}

		bool IsZero() const
		{
			switch (Kind)
			{
			case NumberType_Int8: return i8 == 0;
			case NumberType_UInt8: return u8 == 0;
			case NumberType_Int16: return i16 == 0;
			case NumberType_UInt16: return u16 == 0;
			case NumberType_Int32: return i32 == 0;
			case NumberType_UInt32: return u32 == 0;
			case NumberType_Int64: return i64 == 0;
			case NumberType_UInt64: return u64 == 0;
			case NumberType_Half: return half_ == 0.0f;
			case NumberType_Float: return float_ == 0.0f;
			case NumberType_Double: return double_ == 0.0;
			default: return false;
			}
		}

		bool IsSigned() const noexcept
		{
			switch (Kind)
			{
			case NumberType_Int8:
			case NumberType_Int16:
			case NumberType_Int32:
			case NumberType_Int64:
			case NumberType_Half:
			case NumberType_Float:
			case NumberType_Double:
				return true;
			default:
				return false;
			}
		}

		bool IsNegative() const noexcept
		{
			switch (Kind)
			{
			case NumberType_Int8:
				return i8 < 0;
			case NumberType_Int16:
				return i16 < 0;
			case NumberType_Int32:
				return i32 < 0;
			case NumberType_Int64:
				return i64 < 0;
			case NumberType_Half:
				return half_ < 0 && !half_float::isnan(half_);
			case NumberType_Float:
				return float_ < 0 && !std::isnan(float_);
			case NumberType_Double:
				return double_ < 0 && !std::isnan(double_);
			default:
				return false;
			}
		}

		bool IsIntegral() const noexcept
		{
			switch (Kind)
			{
			case NumberType_UInt8:
			case NumberType_Int8:
			case NumberType_Int16:
			case NumberType_UInt16:
			case NumberType_Int32:
			case NumberType_UInt32:
			case NumberType_Int64:
			case NumberType_UInt64:
				return true;
			default:
				return false;
			}
		}

		size_t ToSizeT() const noexcept
		{
			switch (Kind)
			{
			case NumberType_UInt8:
				return static_cast<size_t>(u8);
			case NumberType_Int8:
				return static_cast<size_t>(i8);
			case NumberType_Int16:
				return static_cast<size_t>(i16);
			case NumberType_UInt16:
				return static_cast<size_t>(u16);
			case NumberType_Int32:
				return static_cast<size_t>(i32);
			case NumberType_UInt32:
				return static_cast<size_t>(u32);
			case NumberType_Int64:
				return static_cast<size_t>(i64);
			case NumberType_UInt64:
				return static_cast<size_t>(u64);
			default:
				return 0;
			}
		}

		std::string ToString() const
		{
			std::ostringstream oss;

			switch (Kind)
			{
			case NumberType_Int8:
				oss << static_cast<int32_t>(i8);
				break;
			case NumberType_UInt8:
				oss << static_cast<uint32_t>(u8);
				break;
			case NumberType_Int16:
				oss << i16;
				break;
			case NumberType_UInt16:
				oss << u16;
				break;
			case NumberType_Int32:
				oss << i32;
				break;
			case NumberType_UInt32:
				oss << u32;
				break;
			case NumberType_Int64:
				oss << i64;
				break;
			case NumberType_UInt64:
				oss << u64;
				break;
			case NumberType_Half:
				oss << static_cast<float>(half_);
				break;
			case NumberType_Float:
				oss << float_;
				break;
			case NumberType_Double:
				oss << double_;
				break;
			default:
				oss << "Unknown";
				break;
			}

			return oss.str();
		}

		uint64_t hash() const noexcept
		{
			XXHash3_64 hash{};

			hash.Combine(Kind);

			switch (Kind)
			{
			case NumberType_UInt8:  hash.Combine(u8);  break;
			case NumberType_Int8:   hash.Combine(i8);  break;
			case NumberType_Int16:  hash.Combine(i16); break;
			case NumberType_UInt16: hash.Combine(u16); break;
			case NumberType_Int32:  hash.Combine(i32); break;
			case NumberType_UInt32: hash.Combine(u32); break;
			case NumberType_Int64:  hash.Combine(i64); break;
			case NumberType_UInt64: hash.Combine(u64); break;
			case NumberType_Half:
				hash.Combine(static_cast<uint16_t>(half_));
				break;
			case NumberType_Float:
				hash.Combine(static_cast<uint32_t>(float_));
				break;
			case NumberType_Double:
				hash.Combine(static_cast<uint64_t>(double_));
				break;
			}

			return hash.Finalize();
		}
	};

	constexpr Number UNKNOWN_NUMBER = {};

	bool ParseNumber(const char* text, size_t length, size_t position, bool isHex, bool isBinary, bool isSigned, Number& number, size_t& tokenLength);
}
#endif