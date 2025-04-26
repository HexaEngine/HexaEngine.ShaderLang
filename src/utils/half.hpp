#ifndef HALF_HPP
#define HALF_HPP

namespace HXSL
{
	class half
	{
	public:
		uint16_t value;

		half() : value(0) {}
		half(float f) {
			// Convert float to half-precision
			uint32_t f_bits = *reinterpret_cast<uint32_t*>(&f);
			uint32_t sign = (f_bits >> 16) & 0x8000; // sign
			uint32_t exponent = ((f_bits >> 23) & 0xFF) - 112; // exponent bias
			uint32_t mantissa = f_bits & 0x7FFFFF; // mantissa

			if (exponent <= 0) {
				value = sign; // too small, return zero
			}
			else if (exponent >= 31) {
				value = sign | 0x7C00; // too large, return inf
			}
			else {
				mantissa >>= 13; // adjust mantissa
				value = sign | (exponent << 10) | mantissa;
			}
		}

		operator float() const {
			// Convert half-precision to float
			uint32_t sign = value & 0x8000;
			uint32_t exponent = (value >> 10) & 0x1F;
			uint32_t mantissa = value & 0x3FF;

			if (exponent == 0 && mantissa == 0) {
				return 0.0f; // zero
			}
			else if (exponent == 31) {
				return sign ? -INFINITY : INFINITY; // infinity
			}
			else {
				exponent += 112; // adjust bias
				mantissa <<= 13; // adjust mantissa
				uint32_t f_bits = (sign << 16) | (exponent << 23) | mantissa;
				return *reinterpret_cast<float*>(&f_bits);
			}
		}

		bool IsNaN() const noexcept
		{
			// In IEEE half, NaN is identified by exponent bits = 0x1F and non-zero mantissa
			return (value & 0x7C00) == 0x7C00 && (value & 0x03FF) != 0;
		}
	};
}

#endif