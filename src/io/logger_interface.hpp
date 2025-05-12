#ifndef LOGGER_INTERFACE_HPP
#define LOGGER_INTERFACE_HPP

#include "logger.hpp"
#include "log_message.h"
#include "lexical/text_span.hpp"
#include "lexical/input_stream.hpp"
#include "io/source_file.hpp"

#include <string>

class LoggerExtension
{
	ILogger* loggerBase;
	std::string stage;
public:
	LoggerExtension(ILogger* logger, const std::string& stage) : loggerBase(logger), stage(stage)
	{
	}

	template <typename... Args>
	void LogFormatted(LogLevel level, const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
	{
		std::string format = "[" + stage + "]: " + message + " (Line: %i, Column: %i)";
		loggerBase->LogFormatted(level, format, std::forward<Args>(args)..., span.line, span.column);
	}

	template <typename... Args>
	void LogCritical(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
	{
		LogFormatted(LogLevel_Critical, message, file, span, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void LogError(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
	{
		LogFormatted(LogLevel_Error, message, file, span, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void LogWarn(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
	{
		LogFormatted(LogLevel_Warn, message, file, span, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void LogInfo(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
	{
		LogFormatted(LogLevel_Info, message, file, span, std::forward<Args>(args)...);
	}
};

#endif