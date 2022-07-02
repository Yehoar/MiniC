#include "Parser.h"

bool Parser::Parse(const char *filename)
{
    this->filename = filename;
    Scanner scanner;
    if (!scanner.Scan(this->filename))
    {
        return false;
    }

    return this->BuildAST();
}

bool Parser::Parse(Scanner &scanner)
{
    this->tokenList = scanner.GetTokenList();
    return this->BuildAST();
}

bool Parser::BuildAST()
{
    if (this->tokenList.empty())
    {
        return false;
    }
    Match(TokenType::NONE); // start of file
    this->tree.root = this->Program();
    Match(TokenType::NONE); // end of file
    return this->FLAG_AST;
}

AST &Parser::GetAST()
{
    return this->tree;
}

Token &Parser::GetToken()
{
    auto idx = (this->listIdx < this->tokenList.size()) ? this->listIdx : (this->tokenList.size() - 1);
    this->listIdx += 1;
    return this->tokenList.at(idx);
}

Token &Parser::PrevToken(int prev)
{
    size_t idx = this->listIdx - prev - 1;
    return this->tokenList.at(idx < 0 ? 0 : idx);
}

Token &Parser::PeekToken(int peek)
{
    size_t idx = this->listIdx + peek - 1;
    if (idx >= this->tokenList.size())
    {
        idx = this->tokenList.size() - 1;
    }
    return this->tokenList.at(idx);
}

void Parser::Match(TokenType expect)
{
#if _DEBUG
    Logger::Debug("%s ", this->curToken.val.c_str());
#endif

    if (!this->curToken.IsTypeOf(expect))
    {
        // Error: unmatch
        this->EmitError("Unexpected Token", this->curToken);
    }
    this->curToken = this->GetToken();
}

void Parser::EmitError(const string &error, Token &tk)
{
    this->FLAG_AST = false;
    const string &str = tk.val;
    Logger::Error("%.*s '%.*s' at (%d,%d)\n",
                  error.size(),
                  error.data(),
                  str.size(),
                  str.data(),
                  tk.row,
                  tk.col);
    Match(tk.type);
}

ASTNodePointer Parser::Program()
{
    // program -> declaration-list
    this->curToken = this->GetToken();
    return DeclList();
}

ASTNodePointer Parser::DeclList()
{
    // declaration-list -> declaration-list declaration | declaration
    ASTNodePointer subTree = nullptr;

    while (!this->curToken.IsTypeOf(TokenType::NONE))
    {
        ASTNodePointer ptr = Decl();
        if (subTree == nullptr)
        {
            subTree = ptr;
        }
        else
        {
            subTree->AddSibling(ptr);
        }
    }
    return subTree;
}

ASTNodePointer Parser::Decl()
{
    // declaration -> var-declaration | fun-declaration
    //读入后续的Token，判断是变量声明还是函数声明

    size_t size = this->tokenList.size() - this->listIdx;

    for (size_t peek = 1; peek <= size; ++peek)
    {
        Token token = this->PeekToken(peek);
        switch (token.type)
        {
        case TokenType::LPAREN:
        {
            return this->FunDecl();
        }
        // case TokenType::ASSIGN:
        case TokenType::SEMI:
        {
            return this->VarDecl();
        }
        case TokenType::NONE:
        {
            return nullptr;
        }
        default:
            break;
        }
    }
    return nullptr;
}

ASTNodePointer Parser::VarDecl()
{
    this->Match(TokenType::INT);
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::VAR_DECL);
    Match(TokenType::ID);
    if (curToken.IsTypeOf(TokenType::LBRACKET)) //声明数组
    {
        Match(TokenType::LBRACKET);
        subTree->AddChild(curToken, StmtType::ARR_LEN);
        Match(TokenType::NUM);
        Match(TokenType::RBRACKET);
        subTree->stmtType = StmtType::ARR_DECL;
    }
    Match(TokenType::SEMI);
    return subTree;
}

ASTNodePointer Parser::FunDecl()
{
    // fun-declaration -> type-specifier ID(params) compound-stmt
    // type-specifier -> int | void

    ASTNodePointer subTree = new ASTNode(StmtType::FUNC_DECL);
    ASTNodePointer retType = nullptr;
    if (curToken.IsTypeOf(TokenType::VOID))
    {
        retType = new ASTNode(curToken, StmtType::RET_VOID);
        Match(TokenType::VOID);
    }
    else if (curToken.IsTypeOf(TokenType::INT))
    {
        retType = new ASTNode(curToken, StmtType::RET_INT);
        Match(TokenType::INT);
    }
    else
    {
        // ERROR: 预期外的返回值类型
        this->EmitError("Invalid Return Type", curToken);
    }

    subTree->token = curToken;
    Match(TokenType::ID);
    Match(TokenType::LPAREN);
    ASTNodePointer params = Params();
    Match(TokenType::RPAREN);
    ASTNodePointer stmts = CompoundStmt();
    subTree->AddChild(retType);
    subTree->AddChild(params);
    subTree->AddChild(stmts);
    return subTree;
}

ASTNodePointer Parser::Params()
{
    // params -> param-list | void
    if (curToken.IsTypeOf(TokenType::VOID))
    {
        Match(TokenType::VOID);
        return new ASTNode(StmtType::PARAM_VOID);
    }
    else if (curToken.IsTypeOf(TokenType::RPAREN))
    {
        // ERROR: 参数列表为空，但没有找到void
        this->EmitError("Miss Params Before", curToken);
        return nullptr;
    }
    return ParamList();
}

ASTNodePointer Parser::ParamList()
{
    // param-list ->param-list, param | param
    // param->type - specifier ID | type - specifier ID[]

    ASTNodePointer subTree = nullptr;
    while (!curToken.IsTypeOf(TokenType::RPAREN) && !curToken.IsTypeOf(TokenType::NONE))
    {
        Match(TokenType::INT);
        ASTNodePointer param = new ASTNode(curToken, StmtType::PARAM_INT);
        Match(TokenType::ID);
        if (curToken.IsTypeOf(TokenType::LBRACKET))
        {
            Match(TokenType::LBRACKET);
            Match(TokenType::RBRACKET);
            param->stmtType = StmtType::PARAM_ARR;
        }

        if (subTree == nullptr)
        {
            subTree = param;
        }
        else
        {
            subTree->AddSibling(param);
        }

        if (curToken.IsTypeOf(TokenType::COMMA))
        {
            Match(TokenType::COMMA);
        }
    }
    return subTree;
}

ASTNodePointer Parser::CompoundStmt()
{
    // compound-stmt -> { local-declarations statement-list }
    // ASTNodePointer subTree = nullptr, decls = nullptr, stmts = nullptr;
    // Match(TokenType::LBRACE);
    // while (!curToken.IsTypeOf(TokenType::RBRACE) && !curToken.IsTypeOf(TokenType::NONE))
    // {
    //     decls = LocalDecls();
    //     stmts = Stmt();

    //     if (subTree == nullptr)
    //     {
    //         subTree = (decls != nullptr) ? decls : stmts;
    //         if (subTree != stmts)
    //         {
    //             subTree->AddSibling(stmts);
    //         }
    //     }
    //     else
    //     {
    //         subTree->AddSibling(decls);
    //         subTree->AddSibling(stmts);
    //     }
    // }
    // Match(TokenType::RBRACE);

    Match(TokenType::LBRACE);
    ASTNodePointer subTree = new ASTNode(StmtType::COMP_STMT);
    ASTNodePointer decls = LocalDecls();
    ASTNodePointer stmts = StmtList();
    subTree->AddChild(decls);
    subTree->AddChild(stmts);
    Match(TokenType::RBRACE);
    return subTree;
}

ASTNodePointer Parser::LocalDecls()
{
    // local-declarations -> local-declarations var-declaration | empty
    ASTNodePointer subTree = nullptr;
    ASTNodePointer t = nullptr;
    while (curToken.IsTypeOf(TokenType::INT))
    {
        t = VarDecl();
        if (subTree == nullptr)
        {
            subTree = t;
        }
        else
        {
            subTree->AddSibling(t);
        }
    }
    return subTree;
}

ASTNodePointer Parser::StmtList()
{
    // statement-list ->statement-list statement | empty
    // return Stmt();
    ASTNodePointer subTree = nullptr;
    ASTNodePointer ptr = nullptr;
    while (!curToken.IsTypeOf(TokenType::RBRACE))
    {
        ptr = Stmt();
        if (subTree == nullptr)
        {
            subTree = ptr;
        }
        else
        {
            subTree->AddSibling(ptr);
        }
    }
    return subTree;
}
ASTNodePointer Parser::Stmt()
{
    // statement -> expression-stmt | compound-stmt | selection-stmt| iteration-stmt | return-stmt
    switch (curToken.type)
    {
    case TokenType::ID:
        return ExpStmt();
    case TokenType::LBRACE:
        return CompoundStmt();
    case TokenType::IF:
        return SelectionStmt();
    case TokenType::WHILE:
        return IterationStmt();
    case TokenType::RETURN:
        return ReturnStmt();
    case TokenType::SEMI:
    {
        Match(TokenType::SEMI); // ; 空语句
        return nullptr;
    }
    case TokenType::INT:
    case TokenType::VOID:
    {
        // Error: 预期之外的声明语句
        this->EmitError("Unexpected Declaration", curToken);
        return nullptr;
    }
    default:
        return nullptr;
    }
}

ASTNodePointer Parser::ExpStmt()
{
    // expression-stmt -> expression;|;
    ASTNodePointer subTree = nullptr;
    if (!curToken.IsTypeOf(TokenType::SEMI))
    {
        subTree = Exp();
    }
    Match(TokenType::SEMI);
    return subTree;
}

ASTNodePointer Parser::Var()
{
    // var -> ID | ID[expression]
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::VAR_CALL);
    Match(TokenType::ID);
    if (curToken.IsTypeOf(TokenType::LBRACKET))
    {
        subTree->stmtType = StmtType::ARR_CALL;
        Match(TokenType::LBRACKET);
        ASTNodePointer t = Exp();
        Match(TokenType::RBRACKET);
        subTree->AddChild(t);
    }
    return subTree;
}

ASTNodePointer Parser::SimpleExp()
{
    // simple-expression -> additive-expression relop additive-expression | additive-expression
    // relop -> <= | < | > | >= | == | !=
    ASTNodePointer subTree = AdditiveExp();
    while (true)
    {
        ASTNodePointer ptr = Operator();
        if (ptr == nullptr)
            break;
        ptr->AddChild(subTree);
        subTree = ptr;

        ptr = AdditiveExp();
        subTree->AddChild(ptr);
    }
    return subTree;
}

ASTNodePointer Parser::Operator()
{
    ASTNodePointer subTree = nullptr;
    switch (curToken.type)
    {
    case TokenType::LE:
    case TokenType::LT:
    case TokenType::GT:
    case TokenType::GE:
    case TokenType::EQ:
    case TokenType::NE:
    {
        subTree = new ASTNode(curToken, StmtType::RELOP);
        Match(curToken.type);
        break;
    }
    case TokenType::PLUS:
    case TokenType::MINUS:
    {
        subTree = new ASTNode(curToken, StmtType::ADDOP);
        Match(curToken.type);
        break;
    }
    case TokenType::TIMES:
    case TokenType::DIVISION:
    {
        subTree = new ASTNode(curToken, StmtType::MULOP);
        Match(curToken.type);
        break;
    }
    default:
        break;
    }
    return subTree;
}

ASTNodePointer Parser::SelectionStmt()
{
    // selection-stmt -> if(expression) statement|if(expression) statement else statement
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::IF_STMT);
    Match(TokenType::IF);

    Match(TokenType::LPAREN);
    ASTNodePointer e = Exp();
    Match(TokenType::RPAREN);

    ASTNodePointer s = Stmt();

    subTree->AddChild(e);
    subTree->AddChild(s);
    if (curToken.IsTypeOf(TokenType::ELSE))
    {
        Match(TokenType::ELSE);
        s = Stmt();
        subTree->AddChild(s);
    }
    return subTree;
}

ASTNodePointer Parser::IterationStmt()
{
    // iteration-stmt -> while(expression) statement
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::ITER_STMT);
    Match(TokenType::WHILE);
    Match(TokenType::LPAREN);
    ASTNodePointer e = Exp();
    Match(TokenType::RPAREN);
    ASTNodePointer s = Stmt();
    subTree->AddChild(e);
    subTree->AddChild(s);
    return subTree;
}

ASTNodePointer Parser::ReturnStmt()
{
    // return-stmt -> return;|return expression;
    // 等价于 return-stmt -> return expression-stmt
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::RET_STMT);
    Match(TokenType::RETURN);
    ASTNodePointer e = ExpStmt();
    subTree->AddChild(e);
    return subTree;
}

ASTNodePointer Parser::Exp()
{
    // expression -> var=expression|simple-expression
    // curToken.type==TokenType::ID
    // First(Exp)=ID ( NUM
    // Follow(Exp)= ; ) ] ,
    ASTNodePointer subTree = nullptr;
    Token next = this->PeekToken();

    switch (next.type)
    {
    case TokenType::LBRACKET: // [
    case TokenType::ASSIGN:   // =
    {
        subTree = Var();
        if (curToken.IsTypeOf(TokenType::ASSIGN))
        {
            ASTNodePointer ptr = new ASTNode(curToken, StmtType::ASSIGN_STMT);
            ptr->AddChild(subTree);
            subTree = ptr;
            Match(TokenType::ASSIGN);
            ptr = Exp();
            subTree->AddChild(ptr);
        }
        else if (curToken.IsTypeOfRelOp() || curToken.IsTypeOfMathOp())
        {

            ASTNodePointer ptr = Operator();
            while (ptr != nullptr)
            {
                ptr->AddChild(subTree);
                subTree = ptr;
                ptr = SimpleExp();
                subTree->AddChild(ptr);
                ptr = Operator();
            }
        }
        break;
    }
    default:
    {
        ASTNodePointer ptr = SimpleExp();
        if (subTree == nullptr)
        {
            subTree = ptr;
        }
        else
        {
            subTree->AddChild(ptr);
        }
        break;
    }
    }
    return subTree;
}

ASTNodePointer Parser::AdditiveExp()
{
    // additive-expression -> additive-expression addop term | term
    // addop -> + | -
    ASTNodePointer subTree = Term();
    while (true)
    {
        switch (curToken.type)
        {
        case TokenType::PLUS:
        case TokenType::MINUS:
        {
            ASTNodePointer ptr = new ASTNode(curToken, StmtType::ADDOP);
            ptr->AddChild(subTree);
            subTree = ptr;
            Match(curToken.type);
            ptr = Term();
            subTree->AddChild(ptr);
            break;
        }
        default:
            return subTree;
        }
    }
}

ASTNodePointer Parser::Term()
{
    // term -> term mulop factor | factor
    // mulop -> * | /
    ASTNodePointer subTree = Factor();

    while (true)
    {
        switch (curToken.type)
        {
        case TokenType::TIMES:
        case TokenType::DIVISION:
        {
            if (subTree == nullptr)
            {
                // Error: 缺少操作数
                this->EmitError("Missing Operand", curToken);
            }
            ASTNodePointer ptr = new ASTNode(curToken, StmtType::MULOP);
            ptr->AddChild(subTree);
            subTree = ptr;
            Match(curToken.type);
            ptr = Factor();
            subTree->AddChild(ptr);
            break;
        }
        default:
            return subTree;
        }
    }
}

ASTNodePointer Parser::Factor()
{
    // factor -> (expression)|var|call|NUM
    ASTNodePointer subTree = nullptr;
    switch (curToken.type)
    {
    case TokenType::LPAREN:
    {
        Match(TokenType::LPAREN);
        subTree = Exp();
        Match(TokenType::RPAREN);
        break;
    }
    case TokenType::NUM:
    {
        subTree = new ASTNode(curToken, StmtType::NUM);
        Match(TokenType::NUM);
        break;
    }
    case TokenType::ID:
    {
        Token token = this->PeekToken();
        if (token.IsTypeOf(TokenType::LPAREN))
        {
            subTree = Call();
        }
        else
        {
            subTree = Var();
        }
        break;
    }
    default:
        break;
    }
    return subTree;
}

ASTNodePointer Parser::Call()
{
    // call -> ID(args)
    ASTNodePointer subTree = new ASTNode(curToken, StmtType::FUNC_CALL);
    Match(TokenType::ID);
    Match(TokenType::LPAREN);
    ASTNodePointer t = Args();
    Match(TokenType::RPAREN);
    subTree->AddChild(t);
    return subTree;
}

ASTNodePointer Parser::Args()
{
    // args->arg - list | empty
    // arg - list->arg - list, expression | expression
    ASTNodePointer subTree = nullptr;
    while (!curToken.IsTypeOf(TokenType::RPAREN) && !curToken.IsTypeOf(TokenType::NONE))
    {
        ASTNodePointer t = Exp();
        if (subTree == nullptr)
        {
            subTree = t;
        }
        else
        {
            subTree->AddSibling(t);
        }
        if (!curToken.IsTypeOf(TokenType::RPAREN))
        {
            Match(TokenType::COMMA);
        }
    }
    return subTree;
}