#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "config.h"
#include "diagnostic_code.hpp"

#include <fmt/core.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

namespace HXSL
{
	class ILogger
	{
	private:
		std::vector<LogMessage> messages;
		bool hasCriticalErrors;
		int errorCount;

	public:
		ILogger() : hasCriticalErrors(false), errorCount(0)
		{
		}

		bool HasCriticalErrors() const noexcept { return hasCriticalErrors; }

		bool HasErrors() const noexcept { return errorCount > 0 || HasCriticalErrors(); }

		int GetErrorCount()  const noexcept { return errorCount; };

		const std::vector<LogMessage>& GetMessages() const noexcept
		{
			return messages;
		}

		void Log(LogLevel level, const std::string& message)
		{
			messages.push_back(std::move(LogMessage(level, message)));

			if (EnableErrorOutput)
			{
				std::cerr << "[" << ToString(level) << "]: " << message << std::endl;
			}

			if (level == LogLevel_Critical)
			{
				hasCriticalErrors = true;
				HXSL_ASSERT(false, message.c_str());
			}
			else if (level == LogLevel_Error)
			{
				errorCount++;
				if (errorCount >= 100)
				{
					Log(LogLevel_Critical, "Too many errors encountered, stopping compilation!");
				}
			}
		}

		void Log(DiagnosticCode code, const std::string& message);

		template <typename T>
		auto convert_to_cstr(T&& arg) -> decltype(std::forward<T>(arg))
		{
			return std::forward<T>(arg);
		}

		const char* convert_to_cstr(const std::string& arg)
		{
			return arg.c_str();
		}

		const char* convert_to_cstr(std::string&& arg)
		{
			return arg.c_str();
		}

		template <typename... Args>
		[[deprecated("Use DiagnosticCode overload")]]
		void LogFormatted(LogLevel level, const std::string& format, Args&&... args)
		{
			const auto size_s = std::snprintf(nullptr, 0, format.data(), convert_to_cstr(args)...);

			if (size_s <= 0)
			{
				return;
			}

			const auto size = static_cast<size_t>(size_s);
			std::string buf;
			buf.resize(size);

			std::snprintf(buf.data(), size_s + 1, format.data(), convert_to_cstr(args)...);

			Log(level, buf);
		}

		template <typename... Args>
		void LogFormattedInternal(LogLevel level, const std::string& format, Args&&... args)
		{
			const auto size_s = std::snprintf(nullptr, 0, format.data(), convert_to_cstr(args)...);

			if (size_s <= 0)
			{
				return;
			}

			const auto size = static_cast<size_t>(size_s);
			std::string buf;
			buf.resize(size);

			std::snprintf(buf.data(), size_s + 1, format.data(), convert_to_cstr(args)...);

			Log(level, buf);
		}

		template <typename... Args>
		void LogFormattedEx(DiagnosticCode code, const std::string& format, Args&&... args)
		{
			std::string formatFinal = code.GetMessage() + format;

			auto view = fmt::string_view(formatFinal);

			auto storedArgs = std::make_tuple(std::forward<Args>(args)...);

			std::string formatted = std::apply(
				[&](const auto&... unpacked) {
					return fmt::vformat(view, fmt::make_format_args(unpacked...));
				},
				storedArgs
			);

			Log(code, formatted);
		}

		~ILogger()
		{
			std::string	 a = "";
			fmt::vformat(a, fmt::make_format_args(""));
			for (auto& message : messages)
			{
				message.~LogMessage();
			}
		}
	};
}

#endif