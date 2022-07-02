/**
 * AST.h
 * 抽象语法树
 *
 */
#ifndef __AST_H__
#define __AST_H__

#include "Token.h"

enum class StmtType
{
    /* 语句类型 */
    INVALID = -1,
    NONE = 0,

    ARR_DECL, // 数组声明语句
    ARR_CALL, // 数组调用
    ARR_LEN,  // 数组长度或下标

    TYPE_SPEC, //类型标识
    VAR_DECL,  //变量声明
    VAR_CALL,

    RET_INT,    // 返回值类型
    RET_VOID,   // 返回值类型
    FUNC_DECL,  // 函数声明
    PARAM_VOID, // 空形参
    PARAM_INT,  // 整型形参
    PARAM_ARR,  // 数组形参

    FUNC_CALL, // 函数调用

    NUM,   // 常数
    RELOP, // 关系运算符
    ADDOP, // +-运算
    MULOP, //*/运算

    IF_STMT,     // 条件选择语句
    ITER_STMT,   //循环语句
    RET_STMT,    //返回语句
    ASSIGN_STMT, //赋值语句
    COMP_STMT,   //复合语句
};

enum class ExpType
{
    /* 表达式返回值类型 */
    INVALID = -1,
    NONE,
    NUM,  // 常数
    INT,  // 整型
    ARR,  // 数组
    VOID, // 空
};

class ASTNode;
class SymNode;
using ASTNodePointer = ASTNode *;
using SymNodePointer = SymNode *;

class ASTNode
{
    /**
     * 抽象语法树结点
     * sibling 兄弟结点，连接同级语句
     * child 子女结点，执行语句构成部分
     */
public:
    Token token;                        // 单词
    StmtType stmtType{StmtType::NONE};  //语句类型
    ExpType expType{ExpType::NONE};     // 表达式类型
    ASTNodePointer sibling{nullptr};    // 同级语句
    ASTNodePointer *child{nullptr};     // 语句构成
    SymNodePointer symbol_ptr{nullptr}; // 符号表结点
    inline static const int MAXCHILD{3};

private:
    int childIdx{0};

public:
    ASTNode();
    ASTNode(Token tk);
    ASTNode(StmtType st);
    ASTNode(Token tk, StmtType st);
    ~ASTNode();
    void InitChild();                     // 初始化子结点数组
    void AddSibling(ASTNodePointer node); // 添加同级语句到末尾
    void AddChild(ASTNodePointer node);   // 添加子结点
    void AddChild(Token tk, StmtType st);
    bool IsTypeOf(const StmtType &st); // 语句类型判断  ==
    bool IsTypeOf(const ExpType &et);  // 表达式类型判断 ==
};

class AST
{
    /**
     * 抽象语法树
     */

public:
    ASTNodePointer root{nullptr};

private:
    inline static const int INDENT{4};

public:
    AST() = default;
    ~AST();
    void PrintTree();
    string ToString();

private:
    void ToString(ASTNodePointer subTree, string &buf, int indent);
    void Destroy(ASTNodePointer tree);
};

#endif