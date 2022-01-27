#ifndef LOGGING_H
#define LOGGING_H

#include "noncopyable.hpp"

#include <iostream>

#define LOG_INFO(logFormat, ...) \
    do { \
        Logger *logger = Logger::instance(); \
        logger->setLogLevel(INFO); \
        char msg[1024] = {0}; \
        snprintf(msg, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(msg); \
    } while (0)

#define LOG_ERROR(logFormat, ...) \
    do { \
        Logger *logger = Logger::instance(); \
        logger->setLogLevel(ERROR); \
        char msg[1024] = {0}; \
        snprintf(msg, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(msg); \
    } while (0)

#define LOG_DEBUG(logFormat, ...) \
    do { \
        Logger *logger = Logger::instance(); \
        logger->setLogLevel(DEBUG); \
        char msg[1024] = {0}; \
        snprintf(msg, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(msg); \
    } while (0)

#define LOG_FATAL(logFormat, ...) \
    do { \
        Logger *logger = Logger::instance(); \
        logger->setLogLevel(FATAL); \
        char msg[1024] = {0}; \
        snprintf(msg, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(msg); \
        exit(-1); \
    } while (0)


enum LoggerLevel
{
    INFO,  //普通信息
    DEBUG, //调试信息
    ERROR, //错误信息
    FATAL, //core错误信息
};

class Logger : noncopyable
{
public:
    static Logger* instance();
    //设置日志级别
    void setLogLevel(LoggerLevel levels);
    //打印日志内容
    void log(const char* msg);

    ~Logger();
private:
    //单例模式，构造函数私有化
    Logger();
private:
    LoggerLevel loglevel_;
};


#endif
