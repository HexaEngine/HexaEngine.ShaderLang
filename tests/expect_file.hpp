#ifndef EXPECT_FILE_HPP
#define EXPECT_FILE_HPP

#include "pch/std.hpp"

#include "io/logger.hpp"

using namespace HXSL;

struct ExpectFileSubContent
{
	std::string input;
	std::string expected;
	std::vector<LogMessage> expectedLogs;
};

struct TestFileContent
{
	std::vector<ExpectFileSubContent> content;
};

static std::string Trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (first == std::string::npos)
	{
		return "";
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

static std::string ToLower(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

static std::optional<LogMessage> ParseLogMessage(const std::string& line)
{
	auto idx = line.find(']');
	std::string levelStr = line.substr(1, idx - 1);
	std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::tolower);

	static const std::unordered_map<std::string, LogLevel> logLevelMap =
	{
		{"verbose", LogLevel_Verbose},
		{"info", LogLevel_Info},
		{"warn", LogLevel_Warn},
		{"error", LogLevel_Error},
		{"critical", LogLevel_Critical}
	};

	auto it = logLevelMap.find(levelStr);
	if (it == logLevelMap.end())
		return std::nullopt;

	LogLevel level = it->second;
	std::string message = line.substr(idx + 2);

	return LogMessage(level, message);
}

static TestFileContent ParseTestFile(const std::string& filePath)
{
	enum class Section { None, Input, AST, Logs };
	Section current = Section::None;

	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return {};
	}

	TestFileContent result;

	ExpectFileSubContent* currentContent = nullptr;

	std::string line;
	while (std::getline(file, line))
	{
		std::string trimmed = Trim(line);
		std::string lower = ToLower(trimmed);

		if (lower == "#in")
		{
			current = Section::Input;
		}
		else if (lower == "#ast")
		{
			current = Section::AST;
		}
		else if (lower == "#logs")
		{
			current = Section::Logs;
		}
		else if (lower == "#case")
		{
			current = Section::None;
			result.content.emplace_back();
			currentContent = &result.content.back();
		}
		else if (!trimmed.empty() && currentContent != nullptr)
		{
			switch (current)
			{
			case Section::Input:
			{
				auto& input = currentContent->input;
				input.append(line);
				input.push_back('\n');
			}
			break;
			case Section::AST:
			{
				auto& astDump = currentContent->expected;
				astDump.append(line);
				astDump.push_back('\n');
			}
			break;
			case Section::Logs:
			{
				auto logMessage = ParseLogMessage(line);
				if (logMessage)
				{
					currentContent->expectedLogs.push_back(*logMessage);
				}
			}
			break;
			default:
				break;
			}
		}
	}

	file.close();

	return result;
}

#endif