#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include "config.h"

#if defined(__cplusplus)
#include <string>
namespace HXSL
{
#endif

	enum LogLevel
	{
		LogLevel_Verbose,
		LogLevel_Info,
		LogLevel_Warn,
		LogLevel_Error,
		LogLevel_Critical
	};

#if defined(__cplusplus)

	static std::string ToString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel_Verbose:
			return "Verbose";
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

#endif

	struct LogMessage
	{
		LogLevel Level;
		char Message[MAX_LOG_LENGTH];

#if defined(__cplusplus)
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

		std::string ToString() const
		{
			return "[" + HXSL::ToString(Level) + "] " + std::string(Message);
		}
#endif
	};

#if defined(__cplusplus)
}
#endif

#endif