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

		std::string GetMessage() const;

		std::string GetCodeString() const;

		static DiagnosticCode Encode(LogLevel level, const std::string& codeId);
	};
}

#endif