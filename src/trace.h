#pragma once

#include <LinkedList.h>
#include <Arduino.h>


enum class DebugLevel
{
    INFO_LEVEL,
    DEBUG_LEVEL,
    ERROR_LEVEL
};

#ifdef DEBUG
#define X_INFO(iMessage, ...) Trace::trace_message(DebugLevel::INFO_LEVEL, iMessage,  ##__VA_ARGS__)
#define X_DEBUG(iMessage, ...) Trace::trace_message(DebugLevel::DEBUG_LEVEL, iMessage, ##__VA_ARGS__)
#define X_ERROR(iMessage, ...) Trace::trace_message(DebugLevel::ERROR_LEVEL, iMessage, ##__VA_ARGS__)
#else
#define X_INFO(...) 
#define X_DEBUG(...) 
#define X_ERROR(...)
#endif


class ILogger
{
public:
    virtual void begin() = 0;
    virtual void log(const char *iMessage, DebugLevel iLevel, size_t length) = 0;
};

class SerialLogger : public ILogger
{
public:
    void begin();
    void log(const char *iMessage, DebugLevel iLevel, size_t length);
};

inline void SerialLogger::begin()
{
    Serial.begin(115200);
}

inline void SerialLogger::log(const char *iMessage, DebugLevel iLevel, size_t length)
{
    String tag;
    if (iLevel == DebugLevel::INFO_LEVEL)
    {
        tag = "[Info]";
    }
    if (iLevel == DebugLevel::DEBUG_LEVEL)
    {
        tag = "[Debug]";
    }
    if (iLevel == DebugLevel::ERROR_LEVEL)
    {
        tag = "[Error]";
    }
    tag.concat(iMessage);
    Serial.println(tag);
}

class Trace
{
public:
    static void trace_message( DebugLevel iLevel, const char *iMessage, ...);
    static void add_logger(ILogger &iLogger);

private:
    static LinkedList<ILogger*> sLoggers;
};

inline void Trace::trace_message(DebugLevel iLevel, const char *iMessage, ...)
{
    ILogger *logger;
    va_list va;
    va_start(va, iMessage);
    static char buffer[1024];
    int len = vsnprintf(buffer, sizeof(buffer), iMessage, va);
    for (int i = 0; i < sLoggers.size(); i++)
    {
        logger = sLoggers.get(i);
        logger->log(buffer, iLevel, len);
    }
}

inline void Trace::add_logger(ILogger &iLogger)
{
    sLoggers.add(&iLogger);
    iLogger.begin();
}

#ifdef DEFINE_ONCE
LinkedList<ILogger*> Trace::sLoggers;
#endif