#include <windows.h>
#include <iostream>
#include <tchar.h>

// 调试程序路径
TCHAR DEBUG_PROGRAM[] = _T("C:\\Path\\To\\DebugProgram.exe");

// 未处理异常过滤器函数
LONG WINAPI UnhandledExceptionFilterFunc(EXCEPTION_POINTERS* exceptionPointers) {
    std::cout << "Unhandled exception occurred. Starting debug program..." << std::endl;

    // 启动调试程序
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(NULL, DEBUG_PROGRAM, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
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

int _tmain(int argc, TCHAR* argv[]) {
    // 设置未处理异常过滤器
    SetUnhandledExceptionFilter(UnhandledExceptionFilterFunc);

    // 故意触发一个异常，以便测试
    int* ptr = nullptr;
    *ptr = 42;

    return 0;
}