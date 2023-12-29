#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

//#include <benchmark/benchmark.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <map>
#include <mutex>


namespace unit {
    namespace log {
        enum class LogLevel {
            INFO,
            DEBUG,
            WARNING,
            ERROR,
            CUSTOM1,
            CUSTOM2
        };

        class LogDefault {
        private:
            std::ofstream logFile;
            std::string logFileName;
            const std::size_t maxFileSize = 1024 * 1024; // 1 MB
            std::mutex logMutex;


        public:
            LogDefault(const std::string &baseFileName) {
                logFileName = baseFileName + "_" + std::to_string(getProcessId());
                rotateLogFile();
            }

            ~LogDefault() {
                logFile.close();
            }

            void rotateLogFile() {
                std::lock_guard<std::mutex> lock(logMutex); // Lock during file rotation
                // std::time_t t = std::time(nullptr);
                // std::stringstream ss;
                // ss << std::put_time(std::gmtime(&t), "%Y%m%d%H%M%S");
                // std::string timestamp = ss.str();
                logFile.open(logFileName + /* "_" + timestamp + */ ".log");
            }

            template<typename... Args>
            void log(const char *fmt, Args... args) {
                log(LogLevel::DEBUG, fmt, args...);
            }
            void log(LogLevel level, const char *fmt, ...) {
                std::lock_guard<std::mutex> lock(logMutex); // Lock during log write

                if (logFile.tellp() > maxFileSize) {
                    rotateLogFile();
                }

                std::time_t t = std::time(nullptr);
                char time_buf[100];
                std::strftime(time_buf, sizeof time_buf, "%D %T", std::gmtime(&t));

                va_list args1;
                va_start(args1, fmt);
                va_list args2;
                va_copy(args2, args1);
                std::vector<char> buf(1 + std::vsnprintf(nullptr, 0, fmt, args1));
                va_end(args1);
                std::vsnprintf(buf.data(), buf.size(), fmt, args2);
                va_end(args2);

                std::stringstream logMessage;
                logMessage << time_buf << " [" << logLevelToString(level) << "]: " << buf.data();

                logFile << logMessage.str() << std::endl;
            }

        private:
            static std::string logLevelToString(LogLevel level) {

                switch (level) {
                    case LogLevel::INFO:
                        return "info";
                    case LogLevel::DEBUG:
                        return "debug";
                    case LogLevel::WARNING:
                        return "warning";
                    case LogLevel::ERROR:
                        return "error";
                    default:
                        return "unknown";
                }
            }

            static pid_t getProcessId() {
                return getpid();
            }
        };

        //const std::map<LogLevel, std::string> LogDefault::customLogLevelStrings = {
        //    {LogLevel::CUSTOM1, "custom1"},
        //    {LogLevel::CUSTOM2, "custom2"}
        //};

        //LogDefault logger("benchmark_log");

        //void BM_WriteLogFile(benchmark::State& state) {
        //    // The actual benchmark loop
        //    for (auto _ : state) {
        //        logger.log(LogLevel::DEBUG, "Benchmark message: %d", state.iterations());
        //    }
        //}

        // Register the benchmark
        //BENCHMARK(BM_WriteLogFile);

        // Run the benchmark
        //BENCHMARK_MAIN();
    }
}

#endif // __LOGGER_HPP__
