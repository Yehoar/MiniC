/**
 * CLI.h
 * 命令行交互的实现
 *
 */

#ifndef __CLI_H__
#define __CLI_H__

#include <iostream>
#include "Scanner.h"
#include "Parser.h"
#include "SymTable.h"
#include "IR.h"
#include "vm.h"

class CLI
{
public:
    inline static const int FLAG_SCAN = 0x000001;    // 打印Token
    inline static const int FLAG_PARSE = 0x000010;   // 打印语法树
    inline static const int FLAG_SYMTAB = 0x000100;  // 打印符号表
    inline static const int FLAG_IR = 0x001000;      // 生成中间代码
    inline static const int FLAG_COMPILE = 0x001111; // 编译
    inline static const int FLAG_RUN = 0x010000;     // 执行
    inline static const int FLAG_TRACE = 0x100000;   // 打印中间过程
    inline static const int FLAG_DEBUG = 0x1000000;  // 调试

public:
    CLI() = default;
    void Parse(int argc, char **argv);              // 解析命令行参数
    void Compile(int flag, const string &filename); // 编译
    void Run(const string &filename);               // 运行
    void Debug(const string &filename);             // 调试
};

#endif
