/**
 * SymTable.h
 * 符号表和类型检查
 *
 * 采用链表实现
 *
 */
#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <vector>
#include <cstring>
#include <string>
#include <string_view>

#include "Token.h"
#include "AST.h"
#include "MCLog.h"

using std::string;
using std::string_view;
using std::vector;

class SymNode;
using SymNodePointer = SymNode *;

class SymNode
{
    // 符号结点
public:
    int memloc{0};                   // 对于全局或函数作用域：空间的分配大小；对于变量、数组、参数：内存中的偏移位置
    string tag;                      // 符号标记，对于函数：F:G:name:RET_TPYE:PARAM_TYPES，对于变量：V:作用域:大小
    TokenPointer token_ptr{nullptr}; // 单词
    SymNodePointer prev{nullptr};    // 前一个符号或作用域
    SymNodePointer next{nullptr};    // 下一个符号或作用域
    SymNodePointer scope{nullptr};   // 当前作用域内的变量声明

public:
    SymNode() : scope(this) {}
    SymNode(const string &t) : tag(t), scope(this) {}
    ~SymNode() {}
    bool IsVar();                     // 是否变量
    bool IsArr();                     // 是否数组
    bool IsFunc();                    // 是否函数
    bool IsBlock();                   // 是否语句块
    bool HasScope();                  // 是否空作用域
    bool IsGlobal();                  // 是否全局
    bool IsParam();                   // 是否参数
    bool IsLocal();                   // 是否局部
    void Insert(SymNodePointer node); // 向当前作用域插入符号，头插入法
    int Allocate(size_t size);        // 分配内存
    string_view GetRType();           //获取返回值类型
    string_view GetPType();           // 获取参数类型
    SymNodePointer GetFP();           // 获取函数的声明
};

class SymTable
{
    /**
     * 符号表
     */
public:
    bool FLAG_SYMTAB{true};
    bool FLAG_TYPECHECK{true};
    SymNodePointer symtab{nullptr};

private:
    inline static const int INDENT{4};
    ASTNodePointer root{nullptr};

public:
    SymTable() = default;
    ~SymTable();
    bool Build(AST &ast);  // 构建符号表
    bool TypeCheck();      //类型检查
    void PrintTable();     // 打印符号表
    void PrintReference(); // 打印符号引用
    string ToString();

private:
    void InitBuiltIn(); // 向符号表加入内建函数
    void TypeCheck(ASTNodePointer subTree);
    void Destroy(SymNodePointer node);
    void PrintTable(SymNodePointer subTable, string &buffer, int indent = 0);
    void PrintReference(ASTNodePointer subTree, string &buffer, int indent = 0);
    SymNodePointer LookUp(const string &tname, SymNodePointer node); // 查找符号
    void Build(ASTNodePointer ast, SymNodePointer node);
    void AddVar(ASTNodePointer subTree, SymNodePointer node, int pn = 0); // 向作用域添加变量，pn是否参数结点，影响内存的偏移
    void AddArr(ASTNodePointer subTree, SymNodePointer node, int pn = 0); // 添加数组
    void AddFunc(ASTNodePointer subTree);                                 // 添加函数
    void EmitSymError(const string &error, Token &tp);                    // 提交错误
    void EmitTypeError(const string &error, Token &tp);
};

#endif