#include "IR.h"

void IR::PrintIR()
{

    Logger::Print("------------------------------\n");
    Logger::Print("IR:\n");
    Logger::Print("------------------------------\n");
    Logger::Print(this->ToString().c_str());
    Logger::Print("------------------------------\n");
}

string IR::ToString()
{
    int nums = qps.size();
    string buffer;
    buffer.reserve(1024 * 10);

    for (auto i = 0; i < nums; ++i)
    {
        auto &q = qps[i];
        switch (q.opt)
        {
        case Quadruple::TYPE_RO:
        {
            buffer.append(to_string(i))
                .append(": ")
                .append(q.iop)
                .append(" ")
                .append(q.addr1)
                .append(",")
                .append(q.addr2)
                .append(",")
                .append(q.addr3);
            break;
        }
        case Quadruple::TYPE_RM:
        {
            buffer.append(to_string(i))
                .append(": ")
                .append(q.iop)
                .append(" ")
                .append(q.addr1)
                .append(",")
                .append(q.addr2)
                .append("(")
                .append(q.addr3)
                .append(")");
            break;
        }
        default:
            break;
        }
        if (!q.comment.empty())
        {
            buffer.append("  ").append(q.comment);
        }
        buffer.append("\n");
    }
    return buffer;
}

void IR::GenIR(AST &ast, SymTable &table)
{
    // 初始化
    fp = table.symtab->memloc + 2;
    // EmitRM("LDC", GP, "0", "0", "Init GP");
    EmitRM("LDC", FP, to_string(fp), "0", "Init FP");
    // EmitRO("ADD", FP, FP, GP);

    int halt = EmitRM("LDC", AC, "?", "0", "Addr To Halt");
    EmitRM("ST", AC, "-2", FP); // 保存出口地址
    EmitRM("ST", FP, "-1", FP); // 占位

    int saveloc = EmitRM("LDC", PC, "?", PC, "Call main"); // 当前的pc在saveloc+1

    this->Gen(ast.root);

    // 调用main
    int target = inst_offset.at("main");
    // qps[saveloc].addr2 = to_string(target - saveloc - 1);
    qps[saveloc].addr2 = to_string(target);
    target = EmitRO("HALT", "0", "0", "0", "Program End");
    // 回填出口
    qps[halt].addr2 = to_string(target);
}

void IR::Gen(ASTNodePointer subTree, bool isAddr)
{
    for (auto ptr = subTree; ptr != nullptr; ptr = ptr->sibling)
    {
        this->GenStmt(ptr, isAddr);
    }
}

void IR::GenStmt(ASTNodePointer subTree, bool isAddr)
{
    if (subTree == nullptr)
    {
        return;
    }

    switch (subTree->stmtType)
    {
    case StmtType::FUNC_DECL:
    {
        GenFunc(subTree);
        break;
    }
    case StmtType::IF_STMT:
    {
        GenIf(subTree);
        break;
    }
    case StmtType::ITER_STMT:
    {
        GenIter(subTree);
        break;
    }
    case StmtType::RET_STMT:
    {
        GenRet(subTree);
        break;
    }
    case StmtType::COMP_STMT:
    {
        Gen(subTree->child[0]);
        Gen(subTree->child[1]);
        break;
    }
    case StmtType::ASSIGN_STMT:
    {
        GenAS(subTree);
        break;
    }
    case StmtType::RELOP:
    case StmtType::MULOP:
    case StmtType::ADDOP:
    {
        GenExp(subTree, isAddr);
        break;
    }
    case StmtType::NUM:
    {
        EmitRM("LDC", AC, subTree->token.val, "0");
        break;
    }
    case StmtType::VAR_CALL:
    {
        auto sptr = subTree->symbol_ptr;
        string op = isAddr ? "LDA" : "LD";
        string r = isAddr ? BP : AC;
        string s = sptr->IsGlobal() ? GP : FP;
        EmitRM(op, r, to_string(sptr->memloc), s);
        break;
    }
    case StmtType::ARR_CALL:
    {
        GenAC(subTree, isAddr);
        break;
    }
    case StmtType::FUNC_CALL:
    {
        GenFC(subTree);
        break;
    }
    default:
        break;
    }
}

void IR::GenFunc(ASTNodePointer subTree)
{
    // 生成函数声明
    if (subTree == nullptr)
    {
        return;
    }
    // 处理参数和变量的空间
    auto child = subTree->child;
    this->inst_offset[subTree->token.val] = qps.size(); // 记录入口位置
    // 预分配空间
    int tmp = fp;
    fp = subTree->symbol_ptr->memloc;
    int saveloc = qps.size();
    Gen(child[2]);
    EmitComment(" <- Ent " + subTree->token.val, saveloc);

    // 如果函数末尾没有return语句，加上
    ASTNodePointer ptr = child[2];
    while (ptr->sibling)
    {
        ptr = ptr->sibling;
    }
    if (!ptr->IsTypeOf(StmtType::RET_STMT))
    {
        GenRet(nullptr);
    }
    fp = tmp;
}

void IR::GenRet(ASTNodePointer subTree)
{
    if (subTree && subTree->child[0])
    {
        // 有返回值
        Gen(subTree->child[0], false);
        EmitComment("Return Value");
    }
    EmitRM("LDC", BP, "0", "0", "Ret: Clear BP");
    EmitRO("ADD", BP, BP, FP, "Ret: Save Current FP To BP");

    EmitRM("LD", FP, "-1", BP, "Restore FP");
    EmitRM("LD", PC, "-2", BP, "Ret");
}

void IR::GenIf(ASTNodePointer subTree)
{
    if (subTree != nullptr)
    {
        auto child = subTree->child;
        int curLoc = 0;
        // if condition
        curLoc = qps.size();
        Gen(child[0], false);
        int cond_f = EmitRM("JEQ", AC, "?", PC, "If: Jump To If-False"); // 转到if-false
        int saveLoc1 = qps.size();
        EmitComment("If-Condition", curLoc);

        // true
        curLoc = qps.size();
        Gen(child[1]);
        int jmp = EmitRM("LDA", PC, "?", PC, "If-True End"); // 转到if-end
        int saveLoc2 = qps.size();

        // false
        curLoc = qps.size();
        qps[cond_f].addr2 = to_string(curLoc - saveLoc1);
        Gen(child[2]);
        if (curLoc != static_cast<int>(qps.size()))
        {
            EmitComment("If-False End");
        }

        // if-end
        curLoc = qps.size();
        qps[jmp].addr2 = to_string(curLoc - saveLoc2);
    }
}

void IR::GenIter(ASTNodePointer subTree)
{
    if (subTree == nullptr)
    {
        return;
    }
    auto child = subTree->child;

    // condition
    int saveLoc, curLoc, fail;
    saveLoc = qps.size();
    Gen(child[0], false);
    fail = EmitRM("JEQ", AC, "?", PC, "Iter: Jump To End");
    EmitComment("Iter-conditon Begin", saveLoc);
    EmitComment("Iter-conditon End", fail);

    // body
    Gen(child[1], false);
    int jmp = EmitRM("LDA", PC, "?", PC, "Jump To Iter Condition");
    curLoc = qps.size();
    qps[fail].addr2 = to_string(curLoc - fail - 1);
    qps[jmp].addr2 = to_string(saveLoc - curLoc);
}

void IR::GenExp(ASTNodePointer subTree, bool isAddr)
{
    // 生成表达式
    if (subTree == nullptr)
    {
        return;
    }

    ASTNodePointer left = subTree->child[0];
    ASTNodePointer right = subTree->child[1];
    string op;

    switch (subTree->stmtType)
    {
    case StmtType::ADDOP:
    {
        // 生成left-part,如果存在
        if (left)
        {
            Gen(left, false);
            EmitRM("ST", AC, to_string(fp++), FP);
        }
        // 生成right-part, 值会自动保存在AC
        Gen(right, false);
        // 读入left-value
        if (left)
        {
            EmitRM("LD", AC1, to_string(--fp), FP);
        }
        else
        {
            // 针对常数 +1,-1,这类情况转换为 0+1，0-1
            EmitRM("LDC", AC1, "0", "0");
        }
        op = (subTree->token.IsTypeOf(TokenType::PLUS)) ? "ADD" : "SUB";
        EmitRO(op, AC, AC1, AC);
        break;
    }
    case StmtType::MULOP:
    {
        Gen(left, false);
        EmitRM("ST", AC, to_string(fp++), FP);
        Gen(right, false);
        EmitRM("LD", AC1, to_string(--fp), FP);
        op = (subTree->token.IsTypeOf(TokenType::TIMES)) ? "MUL" : "DIV";
        EmitRO(op, AC, AC1, AC);
        break;
    }
    case StmtType::RELOP:
    {
        switch (subTree->token.type)
        {
        case TokenType::LT:
        {
            op = "JLT";
            break;
        }
        case TokenType::LE:
        {
            op = "JLE";
            break;
        }
        case TokenType::EQ:
        {
            op = "JEQ";
            break;
        }
        case TokenType::NE:
        {
            op = "JNE";
            break;
        }
        case TokenType::GE:
        {
            op = "JGE";
            break;
        }
        case TokenType::GT:
        {
            op = "JGT";
            break;
        }
        default:
            break;
        }
        Gen(left, false);
        EmitRM("ST", AC, to_string(fp++), FP);
        Gen(right, false);
        EmitRM("LD", AC1, to_string(--fp), FP);

        EmitRO("SUB", AC, AC1, AC);
        EmitRM(op, AC, "2", PC, "Relop");
        EmitRM("LDC", AC, "0", "0", "Relop False: Set AC 0");
        EmitRM("LDA", PC, "1", PC);
        EmitRM("LDC", AC, "1", "0", "Relop True: Set AC 1");
        break;
    }
    default:
        break;
    }
}

void IR::GenAS(ASTNodePointer subTree)
{
    if (subTree == nullptr)
    {
        return;
    }

    auto child = subTree->child;
    // right-value
    Gen(child[1], false);
    // 将值从AC存到栈中
    EmitRM("ST", AC, to_string(fp++), FP);
    // left-value
    Gen(child[0], true);
    EmitRM("LD", AC, to_string(--fp), FP);
    EmitRM("ST", AC, "0", BP, "Assign End ");
}

void IR::GenFC(ASTNodePointer subTree)
{
    if (subTree == nullptr)
    {
        return;
    }
    auto child = subTree->child;
    SymNodePointer sptr = nullptr;
    string reg;
    // 内置函数
    sptr = subTree->symbol_ptr;
    if (sptr->tag == "F:G:input:I:V")
    {
        EmitRO("IN", AC, "0", "0"); //从标准输入流读入一个整型数据到AC
        return;
    }
    else if (sptr->tag == "F:G:output:V:I")
    {
        GenStmt(child[0]);
        EmitRO("OUT", AC, "0", "0"); //输出一个整型数据到标准输出流
        return;
    }
    // 记录栈顶fp
    int top = fp;
    // 从左到右计算实参
    int begin_args = qps.size();
    for (auto arg = child[0]; arg != nullptr; arg = arg->sibling)
    {

        switch (arg->stmtType)
        {
        case StmtType::NUM:
        case StmtType::ADDOP:
        case StmtType::MULOP:
        case StmtType::ARR_CALL:
        case StmtType::FUNC_CALL:
        {
            GenStmt(arg);
            EmitRM("ST", AC, to_string(fp++), FP);
            break;
        }
        case StmtType::VAR_CALL:
        {
            sptr = arg->symbol_ptr;
            if (sptr->IsArr())
            {
                // 传递数组作为参数
                if (sptr->IsGlobal())
                {
                    EmitRM("LDA", BP, to_string(sptr->memloc), GP, "Load Global Arr Addr");
                }
                else if (sptr->IsParam())
                {
                    // 数组参数多次传递
                    EmitRM("LD", BP, to_string(sptr->memloc), FP, "Load Param Arr Addr");
                }
                else
                {
                    EmitRM("LDA", BP, to_string(sptr->memloc), FP, "Load Local Arr Addr");
                }
            }
            else
            {
                // 普通变量传值
                GenStmt(arg, false);
            }
            reg = sptr->IsArr() ? BP : AC;
            EmitRM("ST", reg, to_string(fp++), FP);
            break;
        }
        default:
            break;
        }
    }
    if (child[0])
    {
        EmitComment("Begin Args", begin_args);
    }

    // int loc = EmitRM("LDC", AC, "?", "0", "Call: Load Return Addr"); // 返回地址
    EmitRM("LDC", AC, to_string(qps.size() + 5), "0", "Call: Load Return Addr"); // 返回地址
    EmitRM("ST", AC, to_string(fp++), FP, "Call: Save Ret");                     // 保存返回地址PC  -2
    EmitRM("ST", FP, to_string(fp++), FP, "Call: Save FP");                      // 保存Old FP     -1
    EmitRM("LDA", FP, to_string(fp), FP, "Call:Modify FP");
    // CALL
    EmitRM("LDC", PC, to_string(this->inst_offset[subTree->token.val]), "0", "Call: Jump To" + subTree->token.val);
    // qps[loc].addr2 = to_string(qps.size());

    // 函数调用结束 清理栈内存
    fp = top;
}

void IR::GenAC(ASTNodePointer subTree, bool isAddr)
{
    if (subTree == nullptr)
    {
        return;
    }
    auto child = subTree->child;
    Gen(child[0], false); // 计算下标值,保存在AC
    // 负下标检查
    EmitRM("JGE", AC, "1", PC, "Check Negative Array Offset");
    EmitRO("HALT", "-1", "0", "0", "Shutdown If Offset Is Negative");

    auto sptr = subTree->symbol_ptr;
    if (sptr->IsGlobal())
    {
        EmitRM("LDA", BP, to_string(sptr->memloc), GP);
    }
    else if (sptr->IsParam())
    {
        EmitRM("LD", BP, to_string(sptr->memloc), FP);
    }
    else
    {
        // 局部数组
        EmitRM("LDA", BP, to_string(sptr->memloc), FP);
    }

    EmitRO("ADD", BP, AC, BP); // 计算偏移地址
    if (!isAddr)
    {
        EmitRM("LD", AC, "0", BP); // 将值读入到AC
    }
}

int IR::EmitRO(string op, string r, string s, string t)
{
    qps.push_back({op, r, s, t, Quadruple::TYPE_RO});
    return qps.size() - 1;
}

int IR::EmitRM(string op, string r, string d, string s)
{
    qps.push_back({op, r, d, s, Quadruple::TYPE_RM});
    return qps.size() - 1;
}

int IR::EmitRO(string op, string r, string s, string t, string c)
{
    qps.push_back({op, r, s, t, Quadruple::TYPE_RO});
    EmitComment(c);
    return qps.size() - 1;
}

int IR::EmitRM(string op, string r, string d, string s, string c)
{
    qps.push_back({op, r, d, s, Quadruple::TYPE_RM});
    EmitComment(c);
    return qps.size() - 1;
}

void IR::EmitComment(string c, int ind)
{
    if (ind < 0)
    {
        ind = qps.size() - 1;
    }
    qps.at(ind).comment.append("# ").append(c).append(" ");
}