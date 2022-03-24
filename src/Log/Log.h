#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <ctime>

namespace WRL {

	class Log
	{
	private:
		std::string logFilePath;

	public:
		enum class LogType
		{
			T_INFO = 0,
			T_WARNING = 1,
			T_ERROR = 2
		};

	private:
		bool generateRandomFile();

		std::string getLogTypeString(LogType logType);

		std::string getCurrentTimeString();

		bool write(LogType logType, const std::string& logMessage);

		bool writeLine(LogType logType, const std::string& logMessage);

		bool writeEndOfLine();

		bool writeToFile(LogType logType, const std::string& logMessage);

		bool writeLineToFile(LogType logType, const std::string& logMessage);

		bool writeEndOfLineToFile();

	public:
		Log(const std::string& logFilePath);
		Log() = delete;
		~Log();

		bool logInfo(const std::string& infoMessage);
		bool logWARNING(const std::string& warningMessage);
		bool logError(const std::string& errorMessage);

	};

	std::string getCurrentTimeStringFileName();

}
