#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <tchar.h>
#pragma comment(lib, "DbgHelp")
std::ofstream outfile("example.txt");

// 调试程序路径
TCHAR CommandLine[255];
// 输出线程的调用堆栈信息
void PrintStackTrace(DWORD dwProcessId, DWORD dwThreadId)
{
    CONTEXT context;
    STACKFRAME64 stackFrame;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
    outfile << "Process ID: " << dwProcessId << " Thread ID: " << dwThreadId << " HANDLE_hProcess: " << hProcess << " HANDLE_hThread: " << hThread << std::endl;
    if (hThread == NULL)
    {
        outfile << "Failed to open thread: " << GetLastError() << std::endl;
        return;
    }

    // 获取线程的上下文
    context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hThread, &context))
    {
        outfile << "Failed to get thread context: " << GetLastError() << std::endl;
        CloseHandle(hThread);
        return;
    }

    // 初始化堆栈帧
    memset(&stackFrame, 0, sizeof(STACKFRAME64));
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrPC.Offset = context.Rip; // RIP 寄存器保存了指令指针的地址
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rbp; // RBP 寄存器保存了当前栈帧的基址指针
    stackFrame.AddrStack.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp; // RSP 寄存器保存了栈顶指针

    // 输出调用堆栈信息
    while (StackWalk64(
        IMAGE_FILE_MACHINE_AMD64, // 指定是 64 位程序
        hProcess,                 // 当前进程句柄
        hThread,                  // 当前线程句柄
        &stackFrame,              // 堆栈帧结构体
        &context,                 // 线程上下文
        NULL,                     // 函数表
        SymFunctionTableAccess64, // 获取函数表的函数
        SymGetModuleBase64,       // 获取模块基址的函数
        NULL))                    // 使用默认的加载模块回调函数
    {
        // 输出堆栈帧的地址
        outfile << "Address: " << std::hex << stackFrame.AddrPC.Offset << std::endl;
    }

    CloseHandle(hThread);
}
// 输出线程的调用堆栈信息
int PrintStackTrace2(PEXCEPTION_POINTERS pExceptionPointers)
{
    STACKFRAME64 stackFrame;
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    outfile << "Process ID: " << GetCurrentProcessId() << " Thread ID: " << GetCurrentThreadId() << " HANDLE_hProcess: " << hProcess << " HANDLE_hThread: " << hThread << std::endl;
    SymInitialize(
        hProcess,
        nullptr,
        TRUE);
    CONTEXT context = *(pExceptionPointers->ContextRecord);
    // 初始化堆栈帧
    memset(&stackFrame, 0, sizeof(STACKFRAME64));
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrPC.Offset = context.Rip; // RIP 寄存器保存了指令指针的地址
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rbp; // RBP 寄存器保存了当前栈帧的基址指针
    stackFrame.AddrStack.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp; // RSP 寄存器保存了栈顶指针

    // 输出调用堆栈信息
    while (StackWalk64(
        IMAGE_FILE_MACHINE_AMD64, // 指定是 64 位程序
        hProcess,                 // 当前进程句柄
        hThread,                  // 当前线程句柄
        &stackFrame,              // 堆栈帧结构体
        &context,                 // 线程上下文
        NULL,                     // 函数表
        SymFunctionTableAccess64, // 获取函数表的函数
        SymGetModuleBase64,       // 获取模块基址的函数
        NULL))                    // 使用默认的加载模块回调函数
    {
        TCHAR noBuffer[] = "";
        TCHAR *pSymbolName = noBuffer;
        DWORD64 dwDisplacement = 0;
        DWORD64 dwAddress = stackFrame.AddrPC.Offset;

        TCHAR buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {0};
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
        {
            pSymbolName = pSymbol->Name;
        }

        DWORD dwDisplacement2;
        IMAGEHLP_LINE64 line;
        SymSetOptions(SYMOPT_LOAD_LINES);
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        BOOL getLine = SymGetLineFromAddr64(hProcess, dwAddress, &dwDisplacement2, &line);
        if (getLine)
        {
            // 输出堆栈帧的地址
            outfile << "Address: " << std::hex << stackFrame.AddrPC.Offset << " SymbolNmae: " << pSymbolName << " FileName: " << line.FileName << " : " << std::format("{}", line.LineNumber) << std::endl;
        }
        else
        {
            // 输出堆栈帧的地址
            outfile << "Address: " << std::hex << stackFrame.AddrPC.Offset << " SymbolNmae: " << pSymbolName << std::endl;
        }
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
int setCommandLine()
{
    int intValue = 42;
    float floatValue = 3.14f;

    // 格式化字符串，并将结果存储在字符数组中
    int length = snprintf(CommandLine, sizeof(CommandLine), "ExceptionStackInformationTracking.exe %d", GetCurrentProcessId());

    // 检查格式化是否成功
    if (length >= 0 && length < sizeof(CommandLine))
    {
        // 输出格式化后的字符串
        printf("Formatted string: %s\n", CommandLine);
    }
    else
    {
        // 格式化失败，输出错误消息
        printf("Error: Buffer overflow occurred.\n");
    }
    PrintStackTrace(GetCurrentProcessId(), GetCurrentThreadId());
    return 0;
}
// 未处理异常过滤器函数
LONG WINAPI UnhandledExceptionFilterFunc(EXCEPTION_POINTERS *exceptionPointers)
{
    PrintStackTrace(GetCurrentProcessId(), GetCurrentThreadId());

    // 继续搜索下一个异常过滤器
    return EXCEPTION_CONTINUE_SEARCH;
}
// 未处理异常过滤器函数
LONG WINAPI UnhandledExceptionFilterFunc1(EXCEPTION_POINTERS *exceptionPointers)
{
    std::cout << "Unhandled exception occurred. Starting debug program..." << std::endl;

    // 启动调试程序
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi;
    if (!CreateProcess(NULL, CommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cerr << "Failed to start debug program: " << GetLastError() << std::endl;
        return EXCEPTION_CONTINUE_SEARCH; // 继续搜索下一个异常过滤器
    }

    // 等待调试程序结束
    WaitForSingleObject(pi.hProcess, INFINITE);

    // 关闭调试程序的句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Debug program ended." << std::endl;

    // 继续搜索下一个异常过滤器
    return EXCEPTION_CONTINUE_SEARCH;
}
void exceptionFunction()
{
    // 故意触发一个异常，以便测试
    int *ptr = nullptr;
    *ptr = 42;
}
int _tmain(int argc, TCHAR *argv[])
{
    // setCommandLine();
    // UnhandledExceptionFilterFunc(nullptr);
    //  设置未处理异常过滤器
    //  SetUnhandledExceptionFilter(UnhandledExceptionFilterFunc);

    __try
    {
        exceptionFunction();
    }
    __except (PrintStackTrace2(GetExceptionInformation()))
    {
        return -1;
    }
    return 0;
}