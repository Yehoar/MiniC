/**
 * Token.h
 * Token的数据结构
 *
 */
#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <map>
#include <vector>
#include <string>
#include <string_view>

#include "MCLog.h"
class Token;

using std::map;
using std::string;
using std::string_view;
using std::vector;

using TokenPointer = Token *;

enum class TokenType
{
    INVALID = -1,
    NONE = 0,
    IF,
    ELSE,
    WHILE, // if,else,return,void,int,while
    PLUS,
    MINUS,
    TIMES,
    DIVISION, //+,-,*,/
    LT,
    LE,
    EQ,
    NE,
    GE,
    GT,  // <,<=,==,!=,>=,>
    NOT, // !
    COMMA,
    SEMI,   // , ;
    ASSIGN, // =
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    LPAREN,
    RPAREN, //{},[],()
    ID,
    NUM,

    RETURN, // return 
    VOID,  // void
    INT  // int
};

class Token
{
public:
    TokenType type; // 记号类型
    string val;     // 记号名称
    int row{0};     // 出现的行号
    int col{0};     // 列号

private:
    static const Token KEYWORD_LIST[]; // 保留关键字
    static const Token SYMBOL_LIST[];  // 保留运算符
    static const Token RELOP_LIST[];   // 关系运算符
    static const Token MATHOP_LIST[];  // 四则运算符
    static const Token SEPARATE[];     // () [] {} , ; 分割符

    static const Token PEEK_TOKEN[]; // 双字符的关键字
    static map<TokenType, string_view> translation;

public:
    static bool IsNeedMore(char c);                      // 需要根据下一个字符进行判断，如 =和==, /和/*, *和*/
    static TokenType GetSymbolType(string_view sv);      // 分隔符类型 ,;[]等
    static TokenType GetKeywordType(string_view sv);     // 关键字和标识符
    static string_view GetTokenTranslation(TokenType t); // 类型转string

public:
    Token() : type(TokenType::NONE), val("") {}
    Token(TokenType t, string v) : type(t), val(v) {}
    bool IsTypeOf(const TokenType &type) const;
    bool IsTypeOfMathOp(); // 四则运算
    bool IsTypeOfRelOp();  // 关系运算
    string GetTypeName();  // 记号名称
};

#endif