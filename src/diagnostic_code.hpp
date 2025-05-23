#ifndef DIAGNOSTIC_CODE_HPP
#define DIAGNOSTIC_CODE_HPP

#include <cstdint>
#include <string>

#include "log_message.h"

namespace HXSL
{
	struct DiagnosticCode
	{
		uint64_t value;

		constexpr DiagnosticCode(uint64_t value) : value(value)
		{
		}

		LogLevel GetLogLevel() const
		{
			return static_cast<LogLevel>((value >> 62) + LogLevel_Info);
		}

		void SetLogLevel(LogLevel level)
		{
			value = (value & ~(0x3ull << 62ull)) | (static_cast<uint64_t>(level) << 62ull);
		}

		void ClearLogLevel()
		{
			value &= ~(0x3ull << 62ull);
		}

		std::string GetMessage() const;

		std::string GetCodeString() const;

		static DiagnosticCode Encode(LogLevel level, const std::string& codeId);

		bool operator==(const DiagnosticCode& other) const
		{
			return value == other.value;
		}

		bool operator!=(const DiagnosticCode& other) const
		{
			return !(*this == other);
		}
	};
}

#endif