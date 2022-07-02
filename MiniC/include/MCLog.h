/*
    MCLog.h
    统一消息输出接口
*/

#ifndef __MCLOG_H__
#define __MCLOG_H__

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <string_view>
using std::string_view;

enum class Level
{
    /* 日志打印级别 */
    DETAIL = 0,
    DEBUG,   // 调试
    INFO,    // 一般
    WARNING, // 警告
    ERROR,   // 错误
    SILENT,  // 静默
};

class Logger
{
private:
    inline static FILE *io_out{stdout};
    inline static Level level{Level::INFO};

public:
    static void SetIO(FILE *io);
    static Level SetLogLevel(Level level);
    static int SetUTF8(); // chcp 65001; //使用UTF-8编码

    static int Print(string_view sv);
    static int Print(const char *s, ...);
    static int Debug(const char *s, ...);
    static int Info(const char *s, ...);
    static int Warning(const char *s, ...);
    static int Error(const char *s, ...);

private:
    Logger();
};

#endif