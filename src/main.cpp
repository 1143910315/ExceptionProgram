#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <tchar.h>
#pragma comment(lib, "DbgHelp")
// 输出线程的调用堆栈信息
int PrintStackTrace(PEXCEPTION_POINTERS pExceptionPointers)
{
    // 打开文件输出
    std::ofstream outfile("example.txt");
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    // 输出进程线程信息
    // outfile << "Process ID: " << GetCurrentProcessId() << " Thread ID: " << GetCurrentThreadId() << " HANDLE_hProcess: " << hProcess << " HANDLE_hThread: " << hThread << std::endl;
    // 符号初始化
    SymInitialize(hProcess, nullptr, TRUE);
    CONTEXT *context = pExceptionPointers->ContextRecord;
    // 初始化堆栈帧
    STACKFRAME64 stackFrame = {0};
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrPC.Offset = context->Rip; // RIP 寄存器保存了指令指针的地址
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context->Rbp; // RBP 寄存器保存了当前栈帧的基址指针
    stackFrame.AddrStack.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context->Rsp; // RSP 寄存器保存了栈顶指针

    // 输出调用堆栈信息
    while (StackWalk64(
        IMAGE_FILE_MACHINE_AMD64, // 指定是 64 位程序
        hProcess,                 // 当前进程句柄
        hThread,                  // 当前线程句柄
        &stackFrame,              // 堆栈帧结构体
        context,                  // 线程上下文
        NULL,                     // 函数表
        SymFunctionTableAccess64, // 获取函数表的函数
        SymGetModuleBase64,       // 获取模块基址的函数
        NULL))                    // 使用默认的加载模块回调函数
    {
        DWORD64 dwAddress = stackFrame.AddrPC.Offset;
        // 输出堆栈帧的地址
        outfile << "Address: " << std::hex << dwAddress;

        DWORD64 dwDisplacement = 0;

        TCHAR buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {0};
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
        {
            // 输出函数符号名
            outfile << " SymbolNmae: " << pSymbol->Name;
        }

        DWORD dwDisplacement2;
        IMAGEHLP_LINE64 line;
        SymSetOptions(SYMOPT_LOAD_LINES);
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        if (SymGetLineFromAddr64(hProcess, dwAddress, &dwDisplacement2, &line))
        {
            // 输出文件名及行号
            outfile << " FileName: " << line.FileName << " : " << std::dec << line.LineNumber;
        }
        outfile << std::endl;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
void exceptionFunction()
{
    // 故意触发一个异常，以便测试
    int *ptr = nullptr;
    *ptr = 42;
}
int _tmain(int argc, TCHAR *argv[])
{
    __try
    {
        exceptionFunction();
    }
    __except (PrintStackTrace(GetExceptionInformation()))
    {
        return -1;
    }
    return 0;
}