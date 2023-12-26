#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

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

    static Logger log;
};