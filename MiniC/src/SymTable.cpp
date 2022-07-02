#include "SymTable.h"
#include <stack>
using std::stack;

bool SymNode::IsVar()
{
    return (!this->tag.empty()) && (this->tag[0] == 'V');
}

bool SymNode::IsArr()
{
    return (!this->tag.empty()) && (this->tag[0] == 'A');
}

bool SymNode::IsFunc()
{
    return (!this->tag.empty()) && (this->tag[0] == 'F');
}

bool SymNode::IsBlock()
{
    return (!this->tag.empty()) && (this->tag[0] == 'B');
}

bool SymNode::HasScope()
{
    return (this->scope != nullptr) && (this->scope != this);
}

bool SymNode::IsGlobal()
{
    return (this->prev != nullptr && this->prev->tag == "_global");
}

bool SymNode::IsParam()
{
    return (this->tag.size() > 3 && this->tag[2] == 'P');
}

bool SymNode::IsLocal()
{
    return (this->tag.size() > 3 && this->tag[2] == 'L');
}

void SymNode::Insert(SymNodePointer node)
{
    // 将局部变量插入到scope
    if (node != nullptr && this != node)
    {
        node->next = this->scope;
        node->prev = this;
        this->scope = node;
    }
}

int SymNode::Allocate(size_t size)
{
    size_t m = this->memloc;
    if (this->prev == nullptr || this->IsFunc())
    {
        this->memloc += size;
        return m;
    }
    return this->prev->Allocate(size);
}

string_view SymNode::GetRType()
{
    string_view sv = this->tag;
    size_t pos = sv.find_last_of(":");
    if (pos == string_view::npos)
    {
        return "?";
    }
    return sv.substr(pos - 1, 1);
}

string_view SymNode::GetPType()
{
    string_view sv = this->tag;
    size_t pos = sv.find_last_of(":");
    if (pos == string_view::npos)
    {
        return "?";
    }
    return sv.substr(pos + 1);
}

SymNodePointer SymNode::GetFP()
{
    if (this->IsFunc())
    {
        return this;
    }
    return (this->prev == nullptr) ? nullptr : this->prev->GetFP();
}

SymTable::~SymTable()
{
    this->Destroy(this->symtab);
    this->symtab = nullptr;
}

void SymTable::PrintTable()
{
    string buffer;
    buffer.reserve(1024 * 10);
    buffer.append("-------------------------------------------\n");
    buffer.append("SymTab:      Name      Tag      Memory\n");
    buffer.append("-------------------------------------------\n");
    buffer.append("|---GLOBAL ").append(std::to_string(this->symtab->memloc)).append("\n");
    this->PrintTable(this->symtab, buffer, SymTable::INDENT);
    buffer.append("-------------------------------------------\n");
    Logger::Print(buffer.c_str());
}

void SymTable::PrintReference()
{
    string buffer;
    buffer.reserve(1024 * 10);
    buffer.append("-------------------------------------------\n");
    buffer.append("Symbol Reference: \n");
    buffer.append("-------------------------------------------\n");
    this->PrintReference(this->root, buffer, 0);
    buffer.append("-------------------------------------------\n");
    Logger::Print(buffer.c_str());
}

string SymTable::ToString()
{
    string buffer;
    buffer.reserve(1024 * 10);
    // Symbol Table
    buffer.append("-------------------------------------------\n");
    buffer.append("SymTab:      Name      Tag      Memory\n");
    buffer.append("-------------------------------------------\n");
    buffer.append("|---GLOBAL ").append(std::to_string(this->symtab->memloc)).append("\n");
    this->PrintTable(this->symtab, buffer, SymTable::INDENT);
    buffer.append("-------------------------------------------\n");
    // Symbol Reference
    buffer.append("-------------------------------------------\n");
    buffer.append("Symbol Reference: \n");
    buffer.append("-------------------------------------------\n");
    this->PrintReference(this->root, buffer, 0);
    buffer.append("-------------------------------------------\n");
    return buffer;
}

void SymTable::Destroy(SymNodePointer node)
{
    if (node == nullptr)
    {
        return;
    }
    SymNodePointer guard = node->prev;
    SymNodePointer tmp = nullptr;
    while (node != nullptr && node != guard)
    {
        if (node->HasScope())
        {
            this->Destroy(node->scope);
        }
        tmp = node;
        node = node->next;
#if _DEBUG
        Logger::Debug("%s\n", tmp->tag.c_str());
#endif
        delete tmp;
    }
}

void SymTable::PrintTable(SymNodePointer subTable, string &buffer, int indent)
{
    stack<SymNodePointer> s;
    SymNodePointer ptr = nullptr;
    for (ptr = subTable->scope; (ptr != nullptr && ptr != subTable); ptr = ptr->next)
    {
        s.push(ptr);
    }

    while (!s.empty())
    {
        ptr = s.top();
        s.pop();
        if (!ptr->IsBlock())
        {
            buffer.append(indent, ' ');
            buffer.append("|---");
            buffer.append(ptr->token_ptr->val).append(", ");
            buffer.append(ptr->tag).append(", ");
            buffer.append(std::to_string(ptr->memloc));
            buffer.append("\n");
        }
        if (ptr->HasScope())
        {
            if (ptr->IsBlock())
            {
                // Block
                buffer.append(indent, ' ').append("|--$BLOCK\n");
            }
            this->PrintTable(ptr, buffer, indent + SymTable::INDENT);
        }
    }
}

void SymTable::PrintReference(ASTNodePointer subTree, string &buffer, int indent)
{
    if (subTree == nullptr)
    {
        return;
    }
    auto s_child = subTree->child;
    switch (subTree->stmtType)
    {
    case StmtType::VAR_CALL:
    case StmtType::ARR_CALL:
    case StmtType::FUNC_CALL:
    {
        buffer.append(indent, ' ').append("|---");
        buffer.append(subTree->token.val)
            .append(" (")
            .append(std::to_string(subTree->token.row))
            .append(",")
            .append(std::to_string(subTree->token.col))
            .append(") -> ")
            .append(subTree->symbol_ptr->tag)
            .append(" (")
            .append(std::to_string(subTree->symbol_ptr->token_ptr->row))
            .append(",")
            .append(std::to_string(subTree->symbol_ptr->token_ptr->col))
            .append(")\n");
        break;
    }
    case StmtType::FUNC_DECL:
    {
        buffer.append(indent, ' ').append("|---");
        buffer.append(subTree->token.val);
        if (subTree->symbol_ptr->HasScope())
        {
            buffer.append(":\n");
        }
        else
        {
            buffer.append(":EMPTY\n");
        }
        break;
    }
    default:
        break;
    }

    for (auto i = 0; i < ASTNode::MAXCHILD; ++i)
    {
        this->PrintReference(s_child[i], buffer, SymTable::INDENT);
    }
    this->PrintReference(subTree->sibling, buffer, indent);
}

SymNodePointer SymTable::LookUp(const string &tname, SymNodePointer node)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    TokenPointer tp = nullptr;
    for (auto ptr = node->scope; ptr != nullptr; ptr = ptr->next)
    {
        tp = ptr->token_ptr;
        if (tp && tp->val == tname)
        {
            return ptr;
        }
    }
    return nullptr;
}

bool SymTable::Build(AST &ast)
{
    this->root = ast.root;

    if (this->symtab)
    {
        this->Destroy(this->symtab);
    }
    this->symtab = new SymNode("_global");
    this->InitBuiltIn();
    this->Build(this->root, this->symtab);
    return this->FLAG_SYMTAB;
}

void SymTable::Build(ASTNodePointer ast, SymNodePointer node)
{
    for (auto ptr = ast; ptr != nullptr; ptr = ptr->sibling)
    {
        switch (ptr->stmtType)
        {
        // 符号收集
        case StmtType::VAR_DECL:
        {
            this->AddVar(ptr, node);
            break;
        }
        case StmtType::ARR_DECL:
        {
            this->AddArr(ptr, node);
            break;
        }
        case StmtType::FUNC_DECL:
        {
            if (node != this->symtab)
            {
                // Error: 禁止嵌套声明函数
                this->EmitSymError("Nested declaration", ptr->token);
                break;
            }
            this->AddFunc(ptr);
            break;
        }
        case StmtType::COMP_STMT:
        {
            SymNodePointer block = new SymNode("B:_:_:_");
            block->token_ptr = &(ptr->token);
            block->prev = node;
            block->next = (node->scope == nullptr) ? node : node->scope;
            block->scope = block; // 连接到外部作用域
            for (auto i = 0; i < ASTNode::MAXCHILD; ++i)
            {
                this->Build(ptr->child[i], block);
            }
            if (block->HasScope())
            {
                node->Insert(block);
            }
            else
            {
                delete block;
            }

            break;
        }
        // 符号关联
        case StmtType::VAR_CALL:
        case StmtType::ARR_CALL:
        {
            auto symbol = this->LookUp(ptr->token.val, node);
            if (symbol == nullptr)
            {
                // Error: 未定义符号
                this->EmitSymError("Undefined Symbol", ptr->token);
            }
            ptr->symbol_ptr = symbol;
            this->Build(ptr->child[0], node);
            break;
        }
        case StmtType::FUNC_CALL:
        {
            const string &name = ptr->token.val;
            SymNodePointer symbol = nullptr;
            if (name == "input" || name == "output")
            {
                symbol = this->LookUp(name, this->symtab);
            }
            else
            {
                symbol = this->LookUp(name, node);
            }

            if (symbol == nullptr)
            {
                // Error: 未定义符号
                this->EmitSymError("Undefined Symbol", ptr->token);
            }
            ptr->symbol_ptr = symbol;
            this->Build(ptr->child[0], node);
            break;
        }
        case StmtType::RET_STMT:
        {
            ptr->symbol_ptr = node->GetFP();
            this->Build(ptr->child[0], node);
            break;
        }
        default:
        {
            // 符号关联
            for (int i = 0; i < ASTNode::MAXCHILD; ++i)
            {
                this->Build(ptr->child[i], node);
            }
            break;
        }
        }
    }
}

void SymTable::AddVar(ASTNodePointer subTree, SymNodePointer node, int pn)
{
    if (subTree == nullptr || node == nullptr)
    {
        return;
    }
    SymNodePointer symbol = nullptr;
    TokenPointer token = &(subTree->token);
    symbol = this->LookUp(token->val, node); // 检查符号是否已经存在
    if (symbol != nullptr && symbol == nullptr)
    {
        // Error: 重定义
        this->EmitSymError("Duplicate Definition", subTree->token);
        return;
    }
    symbol = new SymNode();
    symbol->token_ptr = token;
    if (node == this->symtab)
    {
        symbol->tag.append("V:G:"); // 全局变量
    }
    else if (subTree->IsTypeOf(StmtType::PARAM_ARR))
    {
        symbol->tag.append("V:P:"); // 参数
    }
    else
    {
        symbol->tag.append("V:L:"); // 局部变量
    }
    symbol->tag.append(token->val);
    symbol->tag.append(":1");
    symbol->memloc = (pn >= 0) ? node->Allocate(1) : pn;
    node->Insert(symbol);
}

void SymTable::AddArr(ASTNodePointer subTree, SymNodePointer node, int pn)
{
    if (subTree == nullptr || node == nullptr)
    {
        return;
    }
    SymNodePointer symbol = nullptr;
    TokenPointer token = &(subTree->token);
    symbol = this->LookUp(token->val, node); // 检查符号是否已经存在
    if (symbol != nullptr && symbol == nullptr)
    {
        // Error: 重定义
        this->EmitSymError("Duplicate Definition", subTree->token);
        return;
    }

    symbol = new SymNode();
    // 数组长度
    size_t length = 1;
    auto sc = subTree->child;
    if (subTree->IsTypeOf(StmtType::ARR_DECL))
    {
        // 数组声明，分配实际大小
        length = std::stoi(sc[0]->token.val);
        if (node == this->symtab)
        {
            symbol->tag.append("A:G:"); // 全局
        }
        else
        {
            symbol->tag.append("A:L:"); // 局部
        }
        symbol->tag.append(token->val).append(":").append(sc[0]->token.val);
    }
    else
    {
        //函数数组形参，分配一个空间占位
        symbol->tag.append("A:P:").append(token->val).append(":#");
    }

    symbol->token_ptr = token;
    symbol->memloc = (pn >= 0) ? node->Allocate(length) : pn;
    node->Insert(symbol);
}

void SymTable::AddFunc(ASTNodePointer subTree)
{
    if (subTree == nullptr)
    {
        return;
    }
    SymNodePointer symbol = nullptr;
    TokenPointer token = &(subTree->token);
    symbol = this->LookUp(token->val, this->symtab); // 检查符号是否已经存在
    if (symbol != nullptr)
    {
        // Error: 重定义
        this->EmitSymError("Duplicate Definition", subTree->token);
        return;
    }
    symbol = new SymNode();
    symbol->token_ptr = token;
    symbol->tag.append("F:G:").append(token->val).append(":"); // F:G:name:RET_TPYE:PARAM_TYPES
    this->symtab->Insert(symbol);
    subTree->symbol_ptr = symbol;

    // 返回值
    string ret = subTree->child[0]->IsTypeOf(StmtType::RET_INT) ? "I:" : "V:";
    // 参数列表
    int v = 0;
    string params;
    // 计算参数个数
    int pnums = 0;
    for (auto ptr = subTree->child[1]; ptr != nullptr; ptr = ptr->sibling)
    {
        pnums += 1;
    }

    for (auto ptr = subTree->child[1]; ptr != nullptr; ptr = ptr->sibling)
    {
        if (ptr->IsTypeOf(StmtType::PARAM_VOID))
        {
            // 参数为void
            v += 1;
            params.append("V");
        }
        else if (ptr->IsTypeOf(StmtType::PARAM_INT))
        {
            params.append("I");
            this->AddVar(ptr, symbol, -2 - pnums);
        }
        else if (ptr->IsTypeOf(StmtType::PARAM_ARR))
        {
            params.append("A");
            this->AddArr(ptr, symbol, -2 - pnums);
        }
        else
        {
            // Error: 异常的语句
            this->EmitSymError("Invalid Statement", subTree->token);
        }
        --pnums;
    }
    if (v > 1 || (v > 0 && params.size() > 1))
    {
        // Error: 多个void或void和其他类型一起出现
        this->EmitSymError("Wrong Params", subTree->token);
    }
    symbol->tag.append(ret).append(params);

    // 解析函数内部
    this->Build(subTree->child[2], symbol);
}

bool SymTable::TypeCheck()
{
    if (this->FLAG_SYMTAB)
    {
        this->TypeCheck(this->root);
        return this->FLAG_TYPECHECK;
    }
    return false;
}

void SymTable::TypeCheck(ASTNodePointer subTree)
{
    if (subTree == nullptr)
        return;
    for (int i = 0; i < ASTNode::MAXCHILD; ++i)
    {
        TypeCheck(subTree->child[i]);
    }
    switch (subTree->stmtType)
    {
    case StmtType::ASSIGN_STMT:
    {
        // 赋值语句
        auto left = subTree->child[0];
        auto right = subTree->child[1];
        // 左值可修改
        if (left != nullptr && !(left->IsTypeOf(StmtType::ARR_CALL) || left->IsTypeOf(StmtType::VAR_CALL)))
        {
            // Error: 左值必须可修改
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Left Value Must Be Variable", left->token);
        }

        if (right != nullptr && !(right->IsTypeOf(ExpType::INT) || right->IsTypeOf(ExpType::NUM)))
        {
            // Error: 无效的右值
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Invalid Right Value", right->token);
        }
        subTree->expType = (this->FLAG_TYPECHECK) ? ExpType::INT : ExpType::INVALID;
        break;
    }
    case StmtType::ADDOP:
    case StmtType::MULOP:
    case StmtType::RELOP:
    {
        auto left = subTree->child[0];
        auto right = subTree->child[1];
        if (left != nullptr && !(left->IsTypeOf(ExpType::INT) || left->IsTypeOf(ExpType::NUM)))
        {
            // Error: 类型错误
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Invalid Value Type", left->token);
        }

        if (right != nullptr && !(right->IsTypeOf(ExpType::INT) || right->IsTypeOf(ExpType::NUM)))
        {
            // Error: 类型错误
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Invalid Value Type", right->token);
        }
        subTree->expType = (this->FLAG_TYPECHECK) ? ExpType::INT : ExpType::INVALID;
        break;
    }
    case StmtType::ARR_CALL:
    {
        // if (subTree->symbol_ptr == nullptr)
        // {
        //     Logger::Error("Token %s \n", subTree->token.val.c_str());
        // }
        if (!subTree->symbol_ptr->IsArr())
        {
            // Error: 下标类型错误
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Wrong Array Call", subTree->token);
        }
        auto child = subTree->child[0];
        if (child != nullptr && !(child->IsTypeOf(ExpType::INT) || child->IsTypeOf(ExpType::NUM)))
        {
            // Error: 下标类型错误
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Wrong Index Value Type", child->token);
        }
        subTree->expType = (this->FLAG_TYPECHECK) ? ExpType::INT : ExpType::INVALID;
        break;
    }
    case StmtType::FUNC_CALL:
    {
        auto symbol = subTree->symbol_ptr;
        subTree->expType = (symbol->GetRType() == "I") ? ExpType::INT : ExpType::VOID;

        // 实参类型检查
        string actual;
        for (auto ptr = subTree->child[0]; ptr != nullptr; ptr = ptr->sibling)
        {
            switch (ptr->expType)
            {
            case ExpType::INT:
            case ExpType::NUM:
            {
                actual.append("I");
                break;
            }
            case ExpType::ARR:
            {
                actual.append("A");
                break;
            }
            case ExpType::VOID:
            {
                actual.append("V");
                break;
            }
            default:
            {
                // Error: 意外的类型
                actual.append("?");
                // this->FLAG_TYPECHECK = false;
                this->EmitTypeError("Unexpected Value Type", subTree->token);
                break;
            }
            }
        }
        if (actual.empty())
        {
            actual.append("V");
        }
        if (symbol->GetPType() != actual)
        {
            // Error: 参数列表不匹配
            // this->FLAG_TYPECHECK = false;
            this->EmitTypeError("Args Type Not Match", subTree->token);
        }
        break;
    }
    case StmtType::IF_STMT:
    case StmtType::ITER_STMT:
    {
        auto c0 = subTree->child[0];
        if (c0 && !(c0->IsTypeOf(ExpType::INT) || c0->IsTypeOf(ExpType::NUM)))
        {
            // Error: 条件表达式的值必须为Int或NUM
            this->EmitTypeError("Unexpected Condition Type", subTree->token);
        }
        break;
    }
    case StmtType::RET_STMT:
    {
        auto c0 = subTree->child[0];
        if (c0 && !(c0->IsTypeOf(ExpType::INT) || c0->IsTypeOf(ExpType::NUM)))
        {
            // Error: 不允许的返回值类型
            this->EmitTypeError("Invalid Return Value Type", subTree->token);
            break;
        }
        // 返回值类型与声明不匹配
        // printf(subTree->token.val.c_str());
        auto fnode = subTree->symbol_ptr;
        // printf(fnode->tag.c_str());
        auto t = fnode->GetRType() == "V" ? ExpType::VOID : ExpType::INT;
        if ((c0 == nullptr && t != ExpType::VOID) ||
            (c0 != nullptr && t != ExpType::INT))
        {
            this->EmitTypeError("Return Type Does Not Match", subTree->token);
        }

        break;
    }
    case StmtType::VAR_CALL:
    {
        SymNodePointer symbol = subTree->symbol_ptr;
        if (symbol->IsFunc())
        {
            // Error: Invalid Type
            // this->EmitTypeError("Type Error", subTree->token);
            subTree->expType = ExpType::INVALID;
        }
        else
        {
            // if (subTree->symbol_ptr == nullptr)
            // {
            //     Logger::Error("Token %s \n", subTree->token.val.c_str());
            // }
            subTree->expType = (subTree->symbol_ptr->IsArr()) ? ExpType::ARR : ExpType::INT;
        }

        break;
    }
    case StmtType::NUM:
    {
        subTree->expType = ExpType::NUM;
        break;
    }

    default:
        break;
    }
    TypeCheck(subTree->sibling);
}

void SymTable::EmitSymError(const string &error, Token &tp)
{
    this->FLAG_SYMTAB = false;
    Logger::Error("%.*s: \"%.*s\" at (%d,%d)\n",
                  error.size(),
                  error.data(),
                  tp.val.size(),
                  tp.val.data(),
                  tp.row,
                  tp.col);
}

void SymTable::EmitTypeError(const string &error, Token &tp)
{
    this->FLAG_TYPECHECK = false;
    Logger::Error("%.*s: \"%.*s\" at (%d,%d)\n",
                  error.size(),
                  error.data(),
                  tp.val.size(),
                  tp.val.data(),
                  tp.row,
                  tp.col);
}

void SymTable::InitBuiltIn()
{
    // int input(void)
    SymNodePointer f_input = new SymNode("F:G:input:I:V");
    static Token t_input(TokenType::ID, "input");
    f_input->token_ptr = &t_input;

    // void output(int)
    SymNodePointer f_output = new SymNode("F:G:output:V:I");
    static Token t_output(TokenType::ID, "output");
    f_output->token_ptr = &t_output;

    this->symtab->Insert(f_input);
    this->symtab->Insert(f_output);
}
