#include "minic.h"

// 引入minic源码

#include <cstdio>
#include "MiniC/include/Scanner.h"
#include "MiniC/include/Parser.h"
#include "MiniC/include/SymTable.h"
#include "MiniC/include/IR.h"


int Minic::Compile(QString& filename)
{

    if(!filename.endsWith(".mc"))
    {
        return -1;
    }
    string fstr = filename.toStdString();

    // 词法分析
    string tmp = fstr+".token";
    FILE* file_ptr = fopen(tmp.c_str(),"w");
    Logger::SetIO(file_ptr);
    Scanner scanner;
    bool isOk = scanner.Scan(fstr);
    if(isOk)
    {
        fprintf(file_ptr,scanner.ToString().c_str());
    }else
    {
        fclose(file_ptr);
        return -2;
    }
    fclose(file_ptr);

    // 语法分析
    tmp = fstr+".ast";
    file_ptr = fopen(tmp.c_str(),"w");
    Logger::SetIO(file_ptr);
    Parser parser;
    isOk = parser.Parse(scanner);
    if(isOk)
    {
        fprintf(file_ptr, parser.GetAST().ToString().c_str());
    }else
    {
        fclose(file_ptr);
        return -3;
    }
    fclose(file_ptr);

    // 建立符号表和类型检查
    tmp = fstr+".sym";
    file_ptr = fopen(tmp.c_str(),"w");
    Logger::SetIO(file_ptr);
    SymTable table;
    isOk = table.Build(parser.GetAST()) && table.TypeCheck();
    if(isOk)
    {
        fprintf(file_ptr,table.ToString().c_str());
    }else
    {
        fclose(file_ptr);
        return -4;
    }
    fclose(file_ptr);

    // 生成中间代码
    tmp = fstr+".ir";
    file_ptr = fopen(tmp.c_str(),"w");
    Logger::SetIO(file_ptr);
    IR ir;
    ir.GenIR(parser.GetAST(), table);
    fprintf(file_ptr,ir.ToString().c_str());
    fclose(file_ptr);
    Logger::SetIO(stdout);

    return 0;
}

int Minic::Run(QString curPos, QString &filename)
{
    // 检查minic.exe
    QString exe;
    exe.append(curPos).append("/").append("minic.exe");
    QFile file(exe);
    if(!file.open(QFile::ReadOnly))
    {
        file.close();
        return -1;
    }
    file.close();

    // 使用命令行执行中间代码
    QStringList cmd;
    cmd<<"cd /d"<<curPos<<"&& minic.exe -r"<<filename;
    return system(cmd.join(" ").toStdString().c_str());
}
