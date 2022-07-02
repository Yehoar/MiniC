# minic_gui
* 图形界面需要使用QT5.12进行编译
* 命令行执行文件需要cmake+mingw-w64
| // mini_c 图形界面实现   
│  CMakeLists.txt       
│  CMakeLists.txt.user  
│  main.cpp  //图形界面入口  
│  mainwindow.cpp  // 主窗口实现  
│  mainwindow.h         
│  mainwindow.ui   // UI设计  
│  minic.cpp       // minic 接口  
│  minic.h  
│  readme.md  
│
└─MiniC  // minic 编译器 命令行实现  
    │  .gitignore     
    │  CMakeLists.txt  
    │  main.cpp        // minic.exe main入口  
    ├─include  
    │      AST.h    // 抽象语法树的实现  
    │      CLI.h    // 命令行交互  
    │      IR.h     // 中间代码产生器  
    │      MCLog.h  // Logger   
    │      Parser.h  // 语法分析器    
    │      Scanner.h // 词法分析器   
    │      SymTable.h  // 符号表  包括收集符号、类型检查，实际上就是语义分析器   
    │      Token.h  // 记号数据结构  
    │      VM.h   // 虚拟机实现   
    │
    └─src  
            AST.cpp  
            CLI.cpp  
            IR.cpp  
            MCLog.cpp  
            Parser.cpp  
            Scanner.cpp  
            SymTable.cpp  
            Token.cpp  
            VM.cpp  