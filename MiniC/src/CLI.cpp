#include "CLI.h"
#include "MCLog.h"

void CLI::Parse(int argc, char **argv)
{
    const char *arg = nullptr;
    int flag = 0;
    string filename;
    // 解析命令行参数
    for (int i = 1; i < argc; ++i)
    {
        // argv[0] 是源程序
        arg = argv[i];
        if (arg[0] == '-')
        {
            // 参数
            switch (arg[1])
            {
            case 's':
            {
                flag |= FLAG_SCAN;
                break;
            }
            case 'p':
            {
                flag |= FLAG_PARSE;
                break;
            }
            case 't':
            {
                flag |= FLAG_SYMTAB;
                break;
            }
            case 'c':
            {
                flag = (FLAG_SCAN | FLAG_PARSE | FLAG_SYMTAB | FLAG_IR);
                break;
            }
            case 'r':
            {
                flag |= FLAG_RUN;
                break;
            }
            case 'd':
            {
                flag |= FLAG_DEBUG;
                break;
            }
            case 'z':
            {
                flag |= FLAG_TRACE;
                break;
            }
            case 'h':
            {
                Logger::Print("Usage:\n");
                Logger::Print("-s: Show Token List\n");
                Logger::Print("-p: Show Abstract Syntax Tree\n");
                Logger::Print("-t: Show Symbol Table\n");
                Logger::Print("-z: Trace All Step\n");
                Logger::Print("-c: -c <file.mc> Generate IR Code\n");
                Logger::Print("-r: -r <file.ir> Run IR Code\n");
                Logger::Print("-h: Show This Document\n");
                return;
            }
            default:
            {
                flag = -1;
                Logger::Print("Unsupport Argument: %.2s\n", arg);
                return;
            }
            }
        }
        else
        {
            // 输入文件名
            filename = arg;
            string suffixStr = filename.substr(filename.find_last_of('.') + 1);
            if (suffixStr != "ir" && suffixStr != "mc")
            {
                Logger::Print("Unsupport FileType[.mc|.ir]: %s\n", arg);
                return;
            }
        }
    }

    if (!filename.empty())
    {
        if (flag & FLAG_RUN)
        {
            Run(filename);
        }
        else if (flag & FLAG_DEBUG)
        {
            Debug(filename);
        }
        else
        {
            Compile(flag, filename);
        }
    }
    else
    {
        Logger::Print("Please Input Filename\n");
    }
}

void CLI::Compile(int flag, const string &filename)
{
    string suffixStr = filename.substr(filename.size() - 2);
    if (suffixStr != "mc")
    {
        Logger::Print("Unsupported FileType(.mc): %s\n", filename.c_str());
        exit(-1);
    }
    Scanner scanner;
    Parser parser;
    SymTable table;
    IR ir;

    if (flag & FLAG_SCAN)
    {
        if ((scanner.Scan(filename)) && (flag & FLAG_TRACE))
        {
            std::fstream ofs;
            string tmp = scanner.ToString();
            ofs.open(filename + ".token", std::ios::out);
            if (ofs.is_open())
            {
                ofs << tmp << endl;
                ofs.close();
                Logger::Print("# Token List Save At %s.token \n", filename.c_str());
            }
        }
    }

    if (!scanner.FLAG_SCANNER)
    {
        exit(-1);
    }

    if (flag & FLAG_PARSE)
    {
        if ((parser.Parse(scanner)) && (flag & FLAG_TRACE))
        {
            AST &ast = parser.GetAST();
            string tmp = ast.ToString();
            std::fstream ofs;
            ofs.open(filename + ".ast", std::ios::out);
            if (ofs.is_open())
            {
                ofs << tmp << endl;
                ofs.close();
                Logger::Print("# Abstract Syntax Tree Save At %s.ast \n", filename.c_str());
            }
        }
    }

    if (!parser.FLAG_AST)
    {
        exit(-1);
    }

    if (flag & FLAG_SYMTAB)
    {
        AST &ast = parser.GetAST();
        if (table.Build(ast) && table.TypeCheck() && (flag & FLAG_TRACE))
        {
            string tmp = table.ToString();
            std::fstream ofs;
            ofs.open(filename + ".sym", std::ios::out);
            if (ofs.is_open())
            {
                ofs << tmp << endl;
                ofs.close();
                Logger::Print("# Symbol Table Save At %s.sym \n", filename.c_str());
            }
        }
    }

    if (!(table.FLAG_SYMTAB && table.FLAG_TYPECHECK))
    {
        exit(-1);
    }

    if (flag & FLAG_IR)
    {
        AST &ast = parser.GetAST();
        ir.GenIR(ast, table);
        string tmp = ir.ToString();
        std::fstream ofs;
        ofs.open(filename + ".ir", std::ios::out);
        if (ofs.is_open())
        {
            ofs << tmp << endl;
            ofs.close();
        }
        Logger::Print("# IR Code Save At %s.ir \n", filename.c_str());
    }
}

void CLI::Run(const string &filename)
{
    string suffixStr = filename.substr(filename.size() - 2);
    if (suffixStr != "ir")
    {
        Logger::Print("Unsupported FileType(.ir): %s\n", filename.c_str());
        return;
    }
    VM vm;
    vm.LoadInst(filename);
    vm.Run();
    system("pause");
}

void CLI::Debug(const string &filename)
{
    string suffixStr = filename.substr(filename.size() - 2);
    if (suffixStr != "ir")
    {
        Logger::Print("Unsupported FileType(.ir): %s\n", filename.c_str());
        return;
    }
    VM vm;
    vm.LoadInst(filename);
    vm.Debug();
}
