#include "logging.hpp"
#include "timestamp.hpp"
#include <iostream>

Logger::Logger()
    : loglevel_(INFO)
{}

Logger::~Logger()
{}

Logger* Logger::instance()
{
    static Logger logger;
    return &logger;
}

void Logger::setLogLevel(LoggerLevel level)
{
    loglevel_ = level;
}

void Logger::log(const char* msg)
{
    switch(loglevel_) {
    case INFO:
        std::cout << "[INFO]:";
        break;
    case DEBUG:
        std::cout << "[DEBUG]:";
        break;
    case ERROR:
        std::cout << "[ERROR]:";
        break;
    case FATAL:
        std::cout << "[FATAL]:";
        break;
    }
    
    std::cout << Timestamp::now().toString() << "  " << msg << std::endl;
}        
