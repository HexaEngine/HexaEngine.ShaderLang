#ifndef LOG_H
#define LOG_H

#include "config.h"
#include "utils/vector.h"
#include "utils/text_span.h"

#include <stdexcept>
#include <string>
#include <sstream>

namespace HXSL
{
	enum LogLevel
	{
		LogLevel_Info,
		LogLevel_Warn,
		LogLevel_Error,
		LogLevel_Critical
	};

	static std::string ToString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel_Info:
			return "Info";
		case LogLevel_Warn:
			return "Warn";
		case LogLevel_Error:
			return "Error";
		case LogLevel_Critical:
			return "Critical";
		default:
			break;
		}
		return "";
	}

	struct LogMessage
	{
		LogLevel Level;
		char Message[MAX_LOG_LENGTH];

		LogMessage(LogLevel level, const char* message) : Level(level)
		{
			strncpy_s(Message, message, MAX_LOG_LENGTH - 1);
			Message[MAX_LOG_LENGTH - 1] = '\0';
		}

		LogMessage(LogLevel level, const std::string& message) : Level(level)
		{
			strncpy_s(Message, message.c_str(), MAX_LOG_LENGTH - 1);
			Message[MAX_LOG_LENGTH - 1] = '\0';
		}
	};

	class ILogger
	{
	private:
		Vector<LogMessage> messages;
		bool hasCriticalErrors;
		int errorCount;

	public:
		ILogger() : hasCriticalErrors(false), errorCount(0)
		{
		}

		bool HasCriticalErrors() const noexcept { return hasCriticalErrors; }

		bool HasErrors() const noexcept { return errorCount > 0 || HasCriticalErrors(); }

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

		template <typename... Args>
		void LogFormatted(LogLevel level, const std::string& format, Args&&... args)
		{
			const auto size_s = std::snprintf(nullptr, 0, format.data(), std::forward<Args>(args)...);

			if (size_s <= 0)
			{
				return;
			}

			const auto size = static_cast<size_t>(size_s);
			std::string buf;
			buf.resize(size);

			std::snprintf(buf.data(), size_s + 1, format.data(), std::forward<Args>(args)...);

			Log(level, buf);
		}

		~ILogger()
		{
			for (auto& message : messages)
			{
				message.~LogMessage();
			}
		}
	};

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
			loggerBase->LogFormatted(level, format, std::forward<Args>(args)..., span.Line, span.Column);
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
}

#endif
