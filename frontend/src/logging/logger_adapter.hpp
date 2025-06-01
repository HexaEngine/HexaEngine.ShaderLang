#ifndef LOGGER_ADAPTER_HPP
#define LOGGER_ADAPTER_HPP

#include "logging/logger.hpp"
#include "lexical/identifier_table.hpp"

namespace HXSL
{
	class LoggerAdapter
	{
	protected:
		ILogger* logger;
	public:
		LoggerAdapter(ILogger* logger) : logger(logger)
		{
		}

		virtual ~LoggerAdapter() = default;

		ILogger* GetLogger() const noexcept { return logger; }

		template <typename T>
		auto format_arg(T&& arg) const -> decltype(std::forward<T>(arg))
		{
			return std::forward<T>(arg);
		}

		std::string_view format_arg(IdentifierInfo* ptr) const
		{
			return ptr->name.view();
		}

		template <typename... Args>
		void Log(DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			logger->LogFormattedEx(code, span.start, " (Line: {}, Column: {})", format_arg(std::forward<Args>(args))..., span.line, span.column);
		}

		template<typename... Args>
		void Log(DiagnosticCode code, const Token& token, Args&&... args) const
		{
			Log(code, token.Span, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogIf(bool condition, DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			if (condition)
			{
				Log(code, span, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogIf(bool condition, DiagnosticCode code, const Token& token, Args&&... args) const
		{
			LogIf(condition, code, token.Span, std::forward<Args>(args)...);
		}
	};
}

#endif