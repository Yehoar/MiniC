/**
 * Parser.h
 * 语法分析器
 *
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include "Scanner.h"
#include "AST.h"

using std::vector;

class Parser
{
private:
    AST tree;                             // 语法树
    string filename;                      // 文件名
    unsigned int listIdx{0};              // 记录当前Token的位置
    Token curToken{TokenType::NONE, "#"}; // 当前token
    vector<Token> tokenList;              //词法分析器的token列表

private:
    bool BuildAST();
    Token &GetToken();
    Token &PrevToken(int prev = 1); // 向前查看Token
    Token &PeekToken(int peek = 1); // 向后查看Token
    void Match(TokenType expect);
    void EmitError(const string &error, Token &tk);

public:
    bool Parse(const char *filename); // 从指定文件解析语法
    bool Parse(Scanner &scanner);     // 从词法分析器解析语法
    AST &GetAST();                    // 获取语法树
    bool FLAG_AST{true};              // 构建成功标志

private:
    // 递归下降语法分析
    ASTNodePointer Program();
    ASTNodePointer DeclList();
    ASTNodePointer Decl();
    ASTNodePointer VarDecl();
    ASTNodePointer FunDecl();
    ASTNodePointer Params();
    ASTNodePointer ParamList();
    ASTNodePointer CompoundStmt();
    ASTNodePointer LocalDecls();
    ASTNodePointer StmtList();
    ASTNodePointer Stmt();
    ASTNodePointer ExpStmt();
    ASTNodePointer Var();
    ASTNodePointer SimpleExp();
    ASTNodePointer Operator();
    ASTNodePointer SelectionStmt();
    ASTNodePointer IterationStmt();
    ASTNodePointer ReturnStmt();
    ASTNodePointer Exp();
    ASTNodePointer AdditiveExp();
    ASTNodePointer Term();
    ASTNodePointer Factor();
    ASTNodePointer Call();
    ASTNodePointer Args();
};

#endif