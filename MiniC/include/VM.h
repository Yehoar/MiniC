/**
 *  VM.h
 * 中间代码解释器
 *
 */

#ifndef __VM_H__
#define __VM_H__

#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "MCLog.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::map;
using std::string;
using std::string_view;
using std::stringstream;
using std::vector;

enum class OPCODE
{
    /** 
     * RR instructions 
     * op r,s,t
    */

    HALT, // 停机  HALT code,0,0 第一个操作数用作错误代码
    IN,   // 输入 reg[r] = input
    OUT,  // 输出 output reg[r]
    ADD,  // reg[r] = reg[s] op reg[t]
    SUB,  // -
    MUL,  // 
    DIV,
    RRLim,

    /**
     *  RM instructions 
     * op r,d[s]  reg[r] = mem[d + reg[s]]
     */
    LD,  
    ST,  
    RMLim,

    /**
     *  RA instructions 
     *  op r,d[s]  reg[r] = d + reg[s]
     */
    LDA,
    LDC,  
    JLT,
    JLE,
    JEQ,
    JNE,
    JGE,
    JGT,
    RALim,
};

enum class VMSTATUS
{
    /**
     * 解释器状态
     */
    END = 0,                  // 结束
    OK,                       // 正常
    VMError,                  // 出错
    ZeroDivisionError,        // ÷0
    NegativeArrayOffsetError, // 负下标

};

class Instruction
{
    /**
     * 指令
     */
public:
    OPCODE op;
    int arg1;
    int arg2;
    int arg3;
    string raw;  // 原指令

public:
    Instruction() = default;
    Instruction(OPCODE opc, int a1, int a2, int a3) : op(opc), arg1(a1), arg2(a2), arg3(a3) {}
    Instruction(OPCODE opc, int a1, int a2, int a3, string ir) : op(opc), arg1(a1), arg2(a2), arg3(a3), raw(ir) {}
};

class VM
{
public:
    vector<Instruction> instruction;        // 指令列表
    static const map<string, OPCODE> OPMAP; //指令映射 字符串转枚举变量
    int *dMem{nullptr};                     // 内存
    int dm_size{1024};

public: //寄存器
    int Register[8];
    inline static const int REG_AC{0};
    inline static const int REG_AC1{1};
    inline static const int REG_BP{2};
    inline static const int REG_SP{3};
    inline static const int REG_GP{5};
    inline static const int REG_FP{6};
    inline static const int REG_PC{7};

public:
    VM();
    ~VM();
    void LoadInst(const string &filename); // 从文件中读入指令
    void Run();                            // 执行代码
    VMSTATUS RunInst();                    // 执行单句代码
    void Debug();                          // 调试
    void PrintRegister();                  // 打印寄存器和内存
    void PrintError(VMSTATUS e);           // 打印错误
};

#endif