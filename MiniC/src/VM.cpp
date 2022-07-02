#include "VM.h"

const map<string, OPCODE> VM::OPMAP =
    {
        {"HALT", OPCODE::HALT},
        {"IN", OPCODE::IN},
        {"OUT", OPCODE::OUT},
        {"ADD", OPCODE::ADD},
        {"SUB", OPCODE::SUB},
        {"MUL", OPCODE::MUL},
        {"DIV", OPCODE::DIV},
        {"LD", OPCODE::LD},
        {"LDA", OPCODE::LDA},
        {"LDC", OPCODE::LDC},
        {"ST", OPCODE::ST},
        {"JLT", OPCODE::JLT},
        {"JLE", OPCODE::JLE},
        {"JEQ", OPCODE::JEQ},
        {"JNE", OPCODE::JNE},
        {"JGE", OPCODE::JGE},
        {"JGT", OPCODE::JGT},
};

VM::VM()
{
    dMem = new int[dm_size];
    memset(Register, 0, sizeof(Register));
    memset(dMem, 0, dm_size * 4);
}

VM::~VM()
{
    if (this->dMem)
    {
        delete[](this->dMem);
    }
}

void VM::LoadInst(const string &filename)
{
    ifstream ifs;
    ifs.open(filename, std::ios::in);
    if (!ifs.is_open())
    {
        Logger::Error("Can Not Open file: %s \n", filename);
        exit(-1);
    }
    stringstream ss;
    string line, op, tmp;
    int size, a1, a2, a3;
    char f = '\0';
    while (getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        // 解析指令
        ss.str("");
        size = line.size();
        for (int i = 0; i < size; ++i)
        {
            if (!isalnum(line[i]) && line[i] != '-')
            {
                if (line[i] == '#')
                {
                    break;
                }
                if (f != '\0')
                {
                    ss << f;
                    f = '\0';
                }
                continue;
            }
            f = ' ';
            ss << line[i];
        }
        ss >> tmp >> op >> a1 >> a2 >> a3;
        ss.clear();
        instruction.push_back({OPMAP.at(op), a1, a2, a3, line});
    }
    if (ifs.is_open())
    {
        ifs.close();
    }
}

void VM::Run()
{
    int size = instruction.size();
    VMSTATUS ret = VMSTATUS::OK;
    while (ret == VMSTATUS::OK)
    {
        if (Register[REG_PC] < 0 || Register[REG_PC] >= size)
        {
            PrintRegister();
            Logger::Error("PC[%d] Is Out Of Range \n", Register[REG_PC]);
            return;
        }
        ret = RunInst();
    }

    if (ret != VMSTATUS::END) // 程序异常结束
    {
        PrintError(ret);
    }
}

VMSTATUS VM::RunInst()
{
    int r, s, t, m;
    Instruction &inst = instruction.at(Register[REG_PC]);
    Register[REG_PC] += 1;
    if (inst.op < OPCODE::RRLim)
    {
        r = inst.arg1;
        s = inst.arg2;
        t = inst.arg3;
    }
    else
    {
        r = inst.arg1;
        s = inst.arg3;
        m = inst.arg2 + Register[s];
        if ((m < 0 || m > dm_size) && inst.op < OPCODE::RMLim)
        {
            Logger::Error("PC[%d] Is Out Of Range \n", Register[REG_PC]);
            return VMSTATUS::VMError;
        }
    }

    switch (inst.op)
    {
    case OPCODE::HALT:
    {
        switch (r)
        {
        case -1:
            return VMSTATUS::NegativeArrayOffsetError;
        default:
            return VMSTATUS::END;
        }
        break;
    }
    case OPCODE::IN:
    {
        cout << ">>";
        cin >> Register[r];
        break;
    }
    case OPCODE::OUT:
    {
        cout << Register[r] << endl;
        break;
    }
    case OPCODE::ADD:
    {
        Register[r] = Register[s] + Register[t];
        break;
    }
    case OPCODE::SUB:
    {
        Register[r] = Register[s] - Register[t];
        break;
    }
    case OPCODE::MUL:
    {
        Register[r] = Register[s] * Register[t];
        break;
    }
    case OPCODE::DIV:
    {
        if (Register[t] == 0)
        {
            return VMSTATUS::ZeroDivisionError;
        }
        Register[r] = Register[s] / Register[t];
        break;
    }
    case OPCODE::LD:
    {
        Register[r] = dMem[m];
        break;
    }
    case OPCODE::ST:
    {
        dMem[m] = Register[r];
        break;
    }
    case OPCODE::LDC:
    {
        Register[r] = inst.arg2;

        break;
    }
    case OPCODE::LDA:
    {
        Register[r] = m;
        break;
    }
    case OPCODE::JLT:
    {
        if (Register[r] < 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    case OPCODE::JLE:
    {
        if (Register[r] <= 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    case OPCODE::JEQ:
    {
        if (Register[r] == 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    case OPCODE::JNE:
    {
        if (Register[r] != 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    case OPCODE::JGE:
    {
        if (Register[r] >= 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    case OPCODE::JGT:
    {
        if (Register[r] > 0)
        {
            Register[REG_PC] = m;
        }
        break;
    }
    default:
        break;
    }
    return VMSTATUS::OK;
}

void VM::Debug()
{
    int size = instruction.size();

    VMSTATUS ret = VMSTATUS::OK;
    string cmd;
    while (ret == VMSTATUS::OK)
    {

        if (Register[REG_PC] < 0 || Register[REG_PC] >= size)
        {
            PrintRegister();
            Logger::Error("PC[%d] Is Out Of Range \n", Register[REG_PC]);
            return;
        }
        cout << "=============================================" << endl;
        cout << instruction.at(Register[REG_PC]).raw << endl;
        ret = RunInst();
        PrintRegister();
        cout << "=============================================" << endl;
    }
}

void VM::PrintRegister()
{
    Logger::Print("(0)AC:%d (1)AC1:%d (2)BP:%d (5)GP:%d (6)FP:%d (7)PC:%d \n",
                  Register[REG_AC],
                  Register[REG_AC1],
                  Register[REG_BP],
                  Register[REG_GP],
                  Register[REG_FP],
                  Register[REG_PC]);
    Logger::Print("Mem: ");
    for (int i = 0; i < 100; ++i)
    {
        Logger::Print("%d ", dMem[i]);
        if ((i + 1) % 20 == 0)
        {
            Logger::Print("\n     ");
        }
    }
    Logger::Print("\n");
}

void VM::PrintError(VMSTATUS e)
{
    switch (e)
    {
    case VMSTATUS::VMError:
    {
        // Logger::Error("VMError");
        PrintRegister();
        Logger::Error("VMError");
        break;
    }
    case VMSTATUS::NegativeArrayOffsetError:
    {
        Logger::Error("NegativeArrayOffsetError: offset is negative");
        break;
    }
    case VMSTATUS::ZeroDivisionError:
    {
        Logger::Error("ZeroDivisionError: division by zero");
        break;
    }
    default:
        break;
    }
}