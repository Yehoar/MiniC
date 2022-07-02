/**
 * Scanner.h
 * 词法分析器
 *
 */
#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <fstream>
#include <cctype>
#include <string>
#include <queue>
#include <cstring>
#include <vector>

#include "Token.h"
#include "MCLog.h"

using std::ifstream;
using std::queue;
using std::string;
using std::string_view;
using std::to_string;
using std::vector;

// 状态机状态
enum class StateType
{
    START,     // 开始
    DONE,      // 完成
    ERROR,     // 错误
    INCOMMENT, // 注释
    INNUM,     // 常数
    INID,      // 标识符
    MORE       // 查看下一个字符
};

// 词法分析器
class Scanner
{
public:
    bool FLAG_SCANNER{true};  // 状态: true扫描成功

private:
    int row{1};            // 当前字符的行号
    int col{0};            // 当前字符的列号
    int lineLen{0};        // 缓存区的长度
    int linePos{0};        // 缓存区的下标
    int bufferSize{1024};  // 缓存区大小
    char *buffer{nullptr}; // 缓存

    ifstream ifs;            // 文件流
    vector<Token> tokenList; // 保存扫描到的Token序列

public:
    Scanner();
    ~Scanner();
    bool Scan(const string &filename); // 从文件中扫描记号
    vector<Token> &GetTokenList();     // 获取Token列表
    void PrintTokenList();             // 构建
    string ToString();                 // 构建

private:
    bool FillBuffer();  // 填充缓存区
    char GetNextChar(); //获取字符
    void PutBackChar(); // 回退字符
    Token GetToken();   // 获取下一个Token
};

#endif