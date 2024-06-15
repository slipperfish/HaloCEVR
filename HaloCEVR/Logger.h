#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>
#include <sstream>
#include <Windows.h>

class Logger {
	std::ofstream logFile;
	std::mutex mtx;

public:
	Logger(const std::string& filename) {
		logFile.open(filename, std::ios::out | std::ios::trunc);
		if (!logFile) {
			throw std::runtime_error("Unable to open log file");
		}
	}

	~Logger() {
		if (logFile) {
			logFile.close();
		}
	}

	template<typename T>
	Logger& operator<<(const T& msg) {
		std::lock_guard<std::mutex> lock(mtx);
		if (logFile) {
			logFile << msg;
			std::cout << msg;
		}
		return *this;
	}

	Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
		std::lock_guard<std::mutex> lock(mtx);
		if (logFile) {
			logFile << manip;
			std::cout << manip;
		}
		return *this;
	}


	class LoggerAlert {

		std::ostringstream buffer;
		std::string string;
		class Logger* logger;
	public:

		LoggerAlert(Logger* defaultLogger)
		{
			logger = defaultLogger;
		}

		template<typename T>
		LoggerAlert& operator<<(const T& msg) {
			buffer << msg;
			(*logger) << msg;
			return *this;
		}

		LoggerAlert& operator<<(std::ostream& (*manip)(std::ostream&)) {
			buffer << manip;
			(*logger) << manip;

			// detect if endl has been passed into stream here:
			string = buffer.str();

			if (string.find('\n') != std::string::npos)
			{
				MessageBoxA(GetActiveWindow(), string.c_str(), "HaloCEVR Error", MB_OK | MB_ICONERROR);
			}

			return *this;
		}

	};

	static Logger log;
	static LoggerAlert err;
};
