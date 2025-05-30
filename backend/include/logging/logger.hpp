#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "core/config.h"
#include "diagnostic_code.hpp"
#include "utils/interval_tree.hpp"

#include "fmt/core.h"
#include "pch/std.hpp"

namespace HXSL
{
	struct DiagnosticSuppressionRange
	{
		DiagnosticCode code;
		size_t start;
		size_t end;

		explicit DiagnosticSuppressionRange(DiagnosticCode code, size_t start, size_t end) : code(code), start(start), end(end)
		{
		}
	};

	class ILogger
	{
	private:
		std::vector<LogMessage> messages;
		std::vector<DiagnosticSuppressionRange> suppressionRanges;
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

		void AddDiagnosticSuppressionRange(const DiagnosticSuppressionRange& range);

		void Log(LogLevel level, const std::string& message);

		void Log(DiagnosticCode code, size_t location, const std::string& message);

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
		void LogFormattedInternal(LogLevel level, const std::string& format, Args&&... args)
		{
			auto view = fmt::string_view(format);

			auto storedArgs = std::make_tuple(std::forward<Args>(args)...);

			std::string formatted = std::apply(
				[&](const auto&... unpacked) {
					return fmt::vformat(view, fmt::make_format_args(unpacked...));
				},
				storedArgs
			);

			Log(level, formatted);
		}

		template <typename... Args>
		void LogFormattedEx(DiagnosticCode code, size_t location, const std::string& format, Args&&... args)
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

			Log(code, location, formatted);
		}

		~ILogger()
		{
		}
	};
}

#endif