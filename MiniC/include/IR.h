/**
 *  IR.h
 *  生成中间代码
 *  IR.GenIR
 */

#ifndef __IR_H__
#define __IR_H__

#include <map>
#include <sstream>
#include "AST.h"
#include "SymTable.h"

using std::map;
using std::stringstream;
using std::to_string;

// 三地址码(四元式)
class Quadruple
{
public:
    int opt{-1};  // 指令类型 RO/RM
    string iop;   // 操作符
    string addr1; // 地址
    string addr2;
    string addr3;
    string comment; // 注释

public:
    inline static const int TYPE_RO{1}; // 指令类型
    inline static const int TYPE_RM{2};

public:
    Quadruple() : iop("0"), addr1("0"), addr2("0"), addr3("0") {}
    Quadruple(const string &op) : iop(op), addr1("0"), addr2("0"), addr3("0") {}
    Quadruple(const string &op, const string &a1, const string &a2, const string &a3, int t)
        : opt{t}, iop(op), addr1(a1), addr2(a2), addr3(a3) {}
};

class IR
{
public:
    vector<Quadruple> qps; // 保存四元组
    bool FLAG_IR{true};

public:
    // 定义寄存器
    inline static const string AC{"0"};  // 累加器
    inline static const string AC1{"1"}; // 累加器2
    inline static const string BP{"2"};  // 基址寄存器
    // inline static const string SP{"3"};
    inline static const string GP{"5"}; // 全局变量地址寄存器，一般情况下都为0,
    inline static const string FP{"6"}; // 栈帧指针，相当于SP
    inline static const string PC{"7"}; // 程序计数器

private:
    int fp{0};                    // 栈帧指针
    int gp{0};                    // 全局变量
    map<string, int> inst_offset; // 函数指令入口位置

public:
    void PrintIR();
    string ToString();
    void GenIR(AST &ast, SymTable &table);

private:
    void Gen(ASTNodePointer subTree, bool isAddr = false);         // 翻译Program
    void GenStmt(ASTNodePointer subTree, bool isAddr = false);     // 翻译statement
    void GenFunc(ASTNodePointer subTree);                          // 翻译函数声明
    void GenRet(ASTNodePointer subTree);                           // 翻译RETURN语句
    void GenIf(ASTNodePointer subTree);                            // 翻译IF语句
    void GenIter(ASTNodePointer subTree);                          // 翻译WHILE循环语句
    void GenExp(ASTNodePointer subTree, bool isAddr = false);      //翻译表达式
    void GenAS(ASTNodePointer subTree);                            // 翻译赋值表达式
    void GenFC(ASTNodePointer subTree);                            // 翻译函数调用
    void GenAC(ASTNodePointer subTree, bool isAddr = false);       //翻译数组使用
    int EmitRO(string op, string r, string s, string t);           // 保存RO指令
    int EmitRM(string op, string r, string d, string s);           // 保存RM指令
    int EmitRO(string op, string r, string s, string t, string c); // 保存RO指令和注释
    int EmitRM(string op, string r, string d, string s, string c); // 保存RM指令和注释
    void EmitComment(string c, int ind = -1);                      // 添加注释
};

#endif