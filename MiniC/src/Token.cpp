#include "Token.h"

const Token Token::KEYWORD_LIST[] = {
    Token(TokenType::IF, "if"),
    Token(TokenType::ELSE, "else"),
    Token(TokenType::RETURN, "return"),
    Token(TokenType::VOID, "void"),
    Token(TokenType::INT, "int"),
    Token(TokenType::WHILE, "while"),
};

const Token Token::SYMBOL_LIST[] = {
    Token(TokenType::PLUS, "+"),
    Token(TokenType::MINUS, "-"),
    Token(TokenType::TIMES, "*"),
    Token(TokenType::DIVISION, "/"),
    Token(TokenType::NOT, "!"),
    Token(TokenType::LT, "<"),
    Token(TokenType::LE, "<="),
    Token(TokenType::EQ, "=="),
    Token(TokenType::NE, "!="),
    Token(TokenType::GE, ">="),
    Token(TokenType::GT, ">"),
    Token(TokenType::ASSIGN, "="),
};

const Token Token::RELOP_LIST[] = {
    Token(TokenType::LT, "<"),
    Token(TokenType::LE, "<="),
    Token(TokenType::EQ, "=="),
    Token(TokenType::NE, "!="),
    Token(TokenType::GE, ">="),
    Token(TokenType::GT, ">"),
};

const Token Token::MATHOP_LIST[] = {
    Token(TokenType::PLUS, "+"),
    Token(TokenType::MINUS, "-"),
    Token(TokenType::TIMES, "*"),
    Token(TokenType::DIVISION, "/"),
};

const Token Token::PEEK_TOKEN[] = {
    Token(TokenType::LT, "<"),
    Token(TokenType::GT, ">"),
    Token(TokenType::ASSIGN, "="),
    Token(TokenType::NOT, "!"),
};

const Token Token::SEPARATE[] = {
    Token(TokenType::COMMA, ","),
    Token(TokenType::SEMI, ";"),
    Token(TokenType::LBRACE, "{"),
    Token(TokenType::RBRACE, "}"),
    Token(TokenType::LBRACKET, "["),
    Token(TokenType::RBRACKET, "]"),
    Token(TokenType::LPAREN, "("),
    Token(TokenType::RPAREN, ")"),
};

map<TokenType, string_view> Token::translation = {
    {TokenType::INVALID, "INVALID"},
    {TokenType::IF, "KEYWORD"},
    {TokenType::ELSE, "KEYWORD"},
    {TokenType::WHILE, "KEYWORD"},
    {TokenType::RETURN, "KEYWORD"},
    {TokenType::VOID, "KEYWORD"},
    {TokenType::INT, "KEYWORD"},
    {TokenType::PLUS, "ADDOP"},
    {TokenType::MINUS, "ADDOP"},
    {TokenType::TIMES, "MULOP"},
    {TokenType::DIVISION, "MULOP"},
    {TokenType::LT, "RELOP"},
    {TokenType::LE, "RELOP"},
    {TokenType::EQ, "RELOP"},
    {TokenType::NE, "RELOP"},
    {TokenType::GE, "RELOP"},
    {TokenType::GT, "RELOP"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::SEMI, "SEMI"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RBRACE, "RBRACE"},
    {TokenType::LBRACKET, "LBRACKET"},
    {TokenType::RBRACKET, "RBRACKET"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::ID, "ID"},
    {TokenType::NUM, "NUM"},
};

bool Token::IsNeedMore(char c)
{
    for (auto token : Token::PEEK_TOKEN)
    {
        if (token.val[0] == c)
        {
            return true;
        }
    }
    return false;
}

TokenType Token::GetSymbolType(string_view sv)
{
    for (auto token : Token::SYMBOL_LIST)
    {
        if (token.val == sv)
        {
            return token.type;
        }
    }

    for (auto token : Token::SEPARATE)
    {
        if (token.val == sv)
        {
            return token.type;
        }
    }

    return TokenType::INVALID;
}

TokenType Token::GetKeywordType(string_view sv)
{
    for (auto token : Token::KEYWORD_LIST)
    {
        if (token.val == sv)
        {
            return token.type;
        }
    }

    return TokenType::ID;
}

string_view Token::GetTokenTranslation(TokenType t)
{
    auto iter = Token::translation.find(t);
    if (iter != Token::translation.end())
    {
        return iter->second;
    }
    return Token::translation[TokenType::INVALID];
}

bool Token::IsTypeOf(const TokenType &type) const
{
    return this->type == type;
}

bool Token::IsTypeOfMathOp()
{
    for (auto &tk : Token::MATHOP_LIST)
    {
        if (this->IsTypeOf(tk.type))
        {
            return true;
        }
    }
    return false;
}
bool Token::IsTypeOfRelOp()
{
    for (auto &tk : Token::RELOP_LIST)
    {
        if (this->IsTypeOf(tk.type))
        {
            return true;
        }
    }
    return false;
}

string Token::GetTypeName()
{
    auto iter = Token::translation.find(type);
    if (iter != Token::translation.end())
    {
        return string(iter->second.data());
    }
    return "?";
}