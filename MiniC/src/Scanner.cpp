#include "Scanner.h"
#include "MCLog.h"

Scanner::Scanner()
{
    this->buffer = new char[this->bufferSize];
    memset(&buffer[0], 0, this->bufferSize);
}

Scanner::~Scanner()
{
    if (this->buffer != nullptr)
    {
        delete[] this->buffer;
        this->buffer = nullptr;
    }
    ifs.close();
}

bool Scanner::Scan(const string &filename)
{
    this->ifs.open(filename);
    if (!(this->ifs.is_open()))
    {
        Logger::Error("Can Not Open File \"%s\"\n", filename.c_str());
        exit(-1);
    }
    Token t{TokenType::NONE, "SOF"};
    tokenList.push_back(t);
    do
    {
        t = this->GetToken();
        this->tokenList.push_back(t);
    } while (t.type != TokenType::NONE);

    return this->FLAG_SCANNER;
}

bool Scanner::FillBuffer()
{
    if (!ifs.eof())
    {
        this->linePos = 0;
        this->ifs.read(&(this->buffer[0]), this->bufferSize);
        this->lineLen = static_cast<int>(this->ifs.gcount());
        return true;
    }
    return false;
}

char Scanner::GetNextChar()
{
    if (this->linePos >= this->lineLen && !(this->FillBuffer()))
    {
        return 0;
    }
    char c = buffer[linePos++];
    if (c == '\n')
    {
        this->row += 1;
        this->col = 0;
    }
    else
    {
        this->col += 1;
    }

    return c;
}

void Scanner::PutBackChar()
{
    if (this->linePos > 0)
    {
        // 缓冲区指针退回上个字符
        this->linePos -= 1;
    }
    if (this->buffer[this->linePos] == '\n')
    {
        // 退回上一行
        this->row -= 1;
    }
    else if (this->col > 0)
    {
        // 退回前一列
        this->col -= 1;
    }
}

Token Scanner::GetToken()
{
    StateType state = StateType::START;
    Token token;
    while (state != StateType::DONE)
    {
        char nChar = GetNextChar();
        bool save = true;

        switch (state)
        {
        case StateType::START:
        {
            token.row = this->row;
            token.col = this->col;

            if (nChar < 0 || nChar > 255)
            {
                state = StateType::ERROR;
                nChar &= 255;
                if (!ifs.eof() && ifs.fail())
                    ifs.clear();
                this->PutBackChar();
            }
            else if (isalpha(nChar))
            {
                state = StateType::INID;
                token.type = TokenType::ID;
            }
            else if (isdigit(nChar))
            {
                state = StateType::INNUM;
                token.type = TokenType::NUM;
            }
            else if (isspace(nChar))
            {
                save = false;
                break;
            }
            else
            {
                if (nChar == 0)
                {
                    return Token(TokenType::NONE, "EOF");
                }
                state = StateType::MORE;
                token.type = Token::GetSymbolType(string(1, nChar));
            }
            break;
        }
        case StateType::INCOMMENT:
        {
            save = false;
            if (nChar == '*')
            {
                nChar = GetNextChar();
                if (nChar == '/')
                    state = StateType::START;
            }
            break;
        }
        case StateType::INID:
        {
            if (!isalpha(nChar)) // !(isalpha(nChar) || isdigit(nChar))
            {
                save = false;
                PutBackChar();
                state = StateType::DONE;
            }
            break;
        }
        case StateType::INNUM:
        {
            if (!isdigit(nChar))
            {
                save = false;
                PutBackChar();
                state = StateType::DONE;
            }
            break;
        }
        case StateType::MORE:
        {
            state = StateType::DONE;
            if (nChar == 0)
            {
                break;
            }
            if (token.type == TokenType::DIVISION && nChar == '*')
            {
                // 注释 /*
                token.val.clear();
                state = StateType::INCOMMENT;
            }
            else if (Token::IsNeedMore(token.val[0]) && nChar == '=')
            { 
                // 比较运算符
                TokenType t = Token::GetSymbolType(token.val + nChar);
                if (t != TokenType::INVALID)
                {
                    token.type = t;
                    break;
                }
            }
            save = false;
            this->PutBackChar();
            break;
        }
        case StateType::ERROR:
        {
            Logger::Error("Invalid Character: %c (row: %d col: %d )\n", nChar, token.row, token.col);
            state = StateType::DONE;
            token.type = TokenType::INVALID;
            break;
        }
        default:
            break;
        }
        if (save)
            token.val.push_back(nChar);
    }

    if (token.IsTypeOf(TokenType::INVALID))
    {
        Logger::Error("Invalid Token: %s (row: %d col: %d )\n", token.val.c_str(), token.row, token.col);
        this->FLAG_SCANNER = false;
    }

    if (token.IsTypeOf(TokenType::ID))
    {
        token.type = Token::GetKeywordType(token.val);
    }

    return token;
}

vector<Token> &Scanner::GetTokenList()
{
    return this->tokenList;
}

void Scanner::PrintTokenList()
{
    string str = ToString();
    Logger::Print("%.*s\n", str.size(), str.data());
}

string Scanner::ToString()
{
    string str;
    str.reserve(1024 * 10);
    auto begin = tokenList.begin();
    ++begin;
    auto end = tokenList.end();
    --end;
    for (auto iter = begin; iter < end; ++iter)
    {
        auto &token = *iter;
        str.append("row:")
            .append(to_string(token.row))
            .append(",col: ")
            .append(to_string(token.col))
            .append(": ")
            .append(token.GetTypeName())
            .append(": '")
            .append(token.val)
            .append("'\n");
    }
    return str;
}