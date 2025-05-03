#include "logger.hpp"
#include "diagnostic_code.hpp"

namespace HXSL
{
	void ILogger::Log(DiagnosticCode code, const std::string& message)
	{
		auto level = code.GetLogLevel();

		auto fullMessage = code.GetCodeString() + ": " + message;

		messages.push_back(std::move(LogMessage(level, fullMessage)));

		if (EnableErrorOutput)
		{
			std::cerr << "[" << ToString(level) << "] " << fullMessage << std::endl;
		}

		if (level == LogLevel_Critical)
		{
			hasCriticalErrors = true;
			HXSL_ASSERT(false, fullMessage.c_str());
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
}