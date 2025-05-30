#ifndef LOGGER_ADAPTER_HPP
#define LOGGER_ADAPTER_HPP

#include "logging/logger.hpp"

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

		template <typename... Args>
		void Log(DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			logger->LogFormattedEx(code, span.start, " (Line: {}, Column: {})", std::forward<Args>(args)..., span.line, span.column);
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