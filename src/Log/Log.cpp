#include "Log.h"

namespace WRL {

	std::string getCurrentTimeStringFileName() {
		auto currentTime = std::chrono::system_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(currentTime);
		auto timeStructure = localtime(&tt);

		std::string timeString = std::to_string(timeStructure->tm_year + 1900) + " " +
			std::to_string(timeStructure->tm_mon + 1) + "." + std::to_string(timeStructure->tm_mday) + " " + std::to_string(timeStructure->tm_hour) + "." +
			std::to_string(timeStructure->tm_min) + "." + std::to_string(timeStructure->tm_sec);
		return timeString;
	}

	std::string Log::getCurrentTimeString() {
		auto currentTime = std::chrono::system_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(currentTime);
		auto timeStructure = localtime(&tt);

		std::string timeString = std::to_string(timeStructure->tm_year + 1900) + " " +
			std::to_string(timeStructure->tm_mon + 1) + "/" + std::to_string(timeStructure->tm_mday) + " " + std::to_string(timeStructure->tm_hour) + ":" +
			std::to_string(timeStructure->tm_min) + ":" + std::to_string(timeStructure->tm_sec);
		return timeString;
	}

	bool Log::generateRandomFile() {
		std::string timeString = getCurrentTimeString();

		// TODO

		return true;
	}

	std::string Log::getLogTypeString(LogType logType) {
		switch (logType) {
			case LogType::T_INFO:
			{
				std::string str("INFO");
				return str;
				break;
			}
			case LogType::T_WARNING:
			{
				std::string str("WARNING");
				return str;
				break;
			}
			case LogType::T_ERROR:
			{
				std::string str("ERROR");
				return str;
				break;
			}
			default:
			{
				std::string str("NONE");
				return str;
				break;
			}
		}
	}

	Log::Log(const std::string& logFilePath) {
		this->logFilePath = logFilePath;
	}

	Log::~Log() {

	}

	bool Log::write(LogType logType, const std::string& logMessage) {
		std::string combinedLogMessage = "[" + getCurrentTimeString() + "] [" + getLogTypeString(logType) + "] " + logMessage;
		switch (logType) {
			case LogType::T_INFO:
			{
				std::clog << combinedLogMessage;
				break;
			}
			case LogType::T_WARNING:
			{
				std::clog << combinedLogMessage;
				break;
			}
			case LogType::T_ERROR:
			{
				std::cerr << combinedLogMessage;
				break;
			}
			default:
			{
				std::clog << combinedLogMessage;
				break;
			}
		}
		return true;
	}

	bool Log::writeLine(LogType logType, const std::string& logMessage) {
		std::string combinedLogMessage = "[" + getCurrentTimeString() + "] [" + getLogTypeString(logType) + "] " + logMessage;

		switch (logType) {
			case LogType::T_INFO:
			{
				std::clog << combinedLogMessage << std::endl;
				break;
			}
			case LogType::T_WARNING:
			{
				std::clog << combinedLogMessage << std::endl;
				break;
			}
			case LogType::T_ERROR:
			{
				std::cerr << combinedLogMessage << std::endl;
				break;
			}
			default:
			{
				std::clog << combinedLogMessage << std::endl;
				break;
			}
		}
		return true;
	}

	bool Log::writeEndOfLine() {
		std::clog << std::endl;
		return true;
	}

	bool Log::writeToFile(LogType logType, const std::string& logMessage) {
		std::string combinedLogMessage = "[" + getCurrentTimeString() + "] [" + getLogTypeString(logType) + "] " + logMessage;

		std::ofstream outfile;
		outfile.open(this->logFilePath, std::ios::out | std::ios::app);
		if (!outfile.is_open()) {
			return false;
		}

		switch (logType) {
			case LogType::T_INFO:
			{
				std::clog << combinedLogMessage;
				outfile << combinedLogMessage;
				break;
			}
			case LogType::T_WARNING:
			{
				std::clog << combinedLogMessage;
				outfile << combinedLogMessage;
				break;
			}
			case LogType::T_ERROR:
			{
				std::cerr << combinedLogMessage;
				outfile << combinedLogMessage;
				break;
			}
			default:
			{
				std::clog << combinedLogMessage;
				outfile << combinedLogMessage;
				break;
			}
		}
		outfile.close();
		return true;
	}

	bool Log::writeLineToFile(LogType logType, const std::string& logMessage) {
		std::string combinedLogMessage = "[" + getCurrentTimeString() + "] [" + getLogTypeString(logType) + "] " + logMessage;

		std::ofstream outfile;
		outfile.open(this->logFilePath, std::ios::out | std::ios::app);
		if (!outfile.is_open()) {
			return false;
		}

		switch (logType) {
			case LogType::T_INFO:
			{
				std::clog << combinedLogMessage << std::endl;
				outfile << combinedLogMessage << std::endl;
				break;
			}
			case LogType::T_WARNING:
			{
				std::clog << combinedLogMessage << std::endl;
				outfile << combinedLogMessage << std::endl;
				break;
			}
			case LogType::T_ERROR:
			{
				std::cerr << combinedLogMessage << std::endl;
				outfile << combinedLogMessage << std::endl;
				break;
			}
			default:
			{
				std::clog << combinedLogMessage << std::endl;
				outfile << combinedLogMessage << std::endl;
				break;
			}
		}
		outfile.close();
		return true;
	}

	bool Log::writeEndOfLineToFile() {
		std::ofstream outfile;
		outfile.open(this->logFilePath, std::ios::out | std::ios::app);
		if (!outfile.is_open()) {
			return false;
		}

		std::clog << std::endl;
		outfile << std::endl;
		outfile.close();

		return true;
	}

	bool Log::logInfo(const std::string& infoMessage) {
		return writeLineToFile(LogType::T_INFO, infoMessage);
	}

	bool Log::logWARNING(const std::string& warningMessage) {
		return writeLineToFile(LogType::T_WARNING, warningMessage);
	}

	bool Log::logError(const std::string& errorMessage) {
		return writeLineToFile(LogType::T_ERROR, errorMessage);
	}

};
