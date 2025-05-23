#include "logger.hpp"
#include "diagnostic_code.hpp"

namespace HXSL
{
	static int64_t BinarySearch(const std::vector<DiagnosticSuppressionRange>& ranges, size_t idx)
	{
		int64_t low = 0;
		int64_t high = (int64_t)ranges.size() - 1;

		while (low <= high)
		{
			int64_t mid = low + (high - low) / 2;
			const auto& m = ranges[mid];

			if (idx >= m.start && idx < m.end)
			{
				return mid;
			}
			else if (idx < m.start)
			{
				high = mid - 1;
			}
			else
			{
				low = mid + 1;
			}
		}

		return ~low;
	}

	void ILogger::AddDiagnosticSuppressionRange(const DiagnosticSuppressionRange& range)
	{
		auto idx = BinarySearch(suppressionRanges, range.start);
		if (idx < 0)
		{
			idx = ~idx;
		}
		auto copy = range;
		suppressionRanges.insert(suppressionRanges.begin() + idx, copy);
	}

	void ILogger::Log(LogLevel level, const std::string& message)
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

	void ILogger::Log(DiagnosticCode code, size_t location, const std::string& message)
	{
		auto idx = BinarySearch(suppressionRanges, location);
		if (idx >= 0 && suppressionRanges[idx].code == code)
		{
			return;
		}

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