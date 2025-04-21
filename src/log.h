#ifndef LOG_H
#define LOG_H

#include "config.h"
#include "vector.h"
#include "text_span.h"

#include <stdexcept>
#include <string>
#include <sstream>

namespace HXSL
{
	enum HXSLLogLevel
	{
		HXSLLogLevel_Info,
		HXSLLogLevel_Warn,
		HXSLLogLevel_Error,
		HXSLLogLevel_Critical
	};

	static std::string ToString(HXSLLogLevel level)
	{
		switch (level)
		{
		case HXSLLogLevel_Info:
			return "Info";
		case HXSLLogLevel_Warn:
			return "Warn";
		case HXSLLogLevel_Error:
			return "Error";
		case HXSLLogLevel_Critical:
			return "Critical";
		default:
			break;
		}
		return "";
	}

	struct HXSLLogMessage
	{
		HXSLLogLevel Level;
		char Message[MAX_LOG_LENGTH];

		HXSLLogMessage(HXSLLogLevel level, const char* message) : Level(level)
		{
			strncpy_s(Message, message, MAX_LOG_LENGTH - 1);
			Message[MAX_LOG_LENGTH - 1] = '\0';
		}

		HXSLLogMessage(HXSLLogLevel level, const std::string& message) : Level(level)
		{
			strncpy_s(Message, message.c_str(), MAX_LOG_LENGTH - 1);
			Message[MAX_LOG_LENGTH - 1] = '\0';
		}
	};

	class ILogger
	{
	private:
		HXSLVector<HXSLLogMessage> messages;
		bool hasCriticalErrors;

	public:
		ILogger() : hasCriticalErrors(false)
		{
		}

		bool HasCriticalErrors() const noexcept { return hasCriticalErrors; }

		void LogMessage(HXSLLogLevel level, const std::string& message)
		{
			messages.push_back(std::move(HXSLLogMessage(level, message)));

			if (EnableErrorOutput)
			{
				std::cerr << "[" << ToString(level) << "]: " << message << std::endl;
			}

			if (level == HXSLLogLevel_Critical)
			{
				hasCriticalErrors = true;
				HXSL_ASSERT(false, message.c_str());
			}
		}

		template <typename... Args>
		void LogFormatted(HXSLLogLevel level, const std::string& format, Args&&... args)
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

			LogMessage(level, buf);
		}

		~ILogger()
		{
			for (auto& message : messages)
			{
				message.~HXSLLogMessage();
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
		void LogFormatted(HXSLLogLevel level, const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
		{
			std::string format = "[" + stage + "]: " + message + " (Line: %i, Column: %i)";
			loggerBase->LogFormatted(level, format, std::forward<Args>(args)..., span.Line, span.Column);
		}

		template <typename... Args>
		void LogCritical(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
		{
			LogFormatted(HXSLLogLevel_Critical, message, file, span, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void LogError(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
		{
			LogFormatted(HXSLLogLevel_Error, message, file, span, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void LogWarn(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
		{
			LogFormatted(HXSLLogLevel_Warn, message, file, span, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void LogInfo(const std::string& message, const std::string& file, const TextSpan& span, Args&&... args) const
		{
			LogFormatted(HXSLLogLevel_Info, message, file, span, std::forward<Args>(args)...);
		}
	};
}

#endif
