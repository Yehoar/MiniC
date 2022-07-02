#include "MCLog.h"

int Logger::Print(string_view sv)
{
    return fprintf(Logger::io_out, "%.*s", static_cast<int>(sv.size()), sv.data());
}

int Logger::Print(const char *s, ...)
{
    int ret = 0;
    va_list args;
    va_start(args, s);
    ret += vfprintf(Logger::io_out, s, args);
    va_end(args);
    return ret;
}

void Logger::SetIO(FILE *io)
{
    Logger::io_out = io;
}

Level Logger::SetLogLevel(Level level)
{
    Level tmp = Logger::level;
    if (level >= Level::SILENT && level <= Level::ERROR)
    {
        Logger::level = level;
    }
    return tmp;
}

int Logger::SetUTF8()
{
    return system("chcp 65001");
}

int Logger::Debug(const char *s, ...)
{
    if (Logger::level > Level::DEBUG)
    {
        return 0;
    }
    int ret = 0;
    ret += fprintf(Logger::io_out, "[DEBUG] ");
    va_list args;
    va_start(args, s);
    ret += vfprintf(Logger::io_out, s, args);
    va_end(args);
    return ret;
}

int Logger::Info(const char *s, ...)
{
    if (Logger::level > Level::INFO)
    {
        return 0;
    }
    int ret = 0;
    ret += fprintf(Logger::io_out, "[INFO] ");
    va_list args;
    va_start(args, s);
    ret += vfprintf(Logger::io_out, s, args);
    va_end(args);
    return ret;
}

int Logger::Warning(const char *s, ...)
{
    if (Logger::level > Level::WARNING)
    {
        return 0;
    }
    int ret = 0;
    fprintf(Logger::io_out, "[WARNING] ");
    va_list args;
    va_start(args, s);
    ret += vfprintf(Logger::io_out, s, args);
    va_end(args);
    return ret;
}

int Logger::Error(const char *s, ...)
{
    if (Logger::level > Level::ERROR)
    {
        return 0;
    }
    int ret = 0;
    ret += fprintf(Logger::io_out, "[ERROR] ");
    va_list args;
    va_start(args, s);
    ret += vfprintf(Logger::io_out, s, args);
    va_end(args);
    return ret;
}
