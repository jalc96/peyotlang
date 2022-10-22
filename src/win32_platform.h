#include<windows.h>

internal void *allocate_memory(memory_size size) {
    void *result = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    assert(result, "windows memory allocation failed");
    return result;
}

internal void free_memory(void *memory) {
    BOOL result = VirtualFree(memory, 0, MEM_RELEASE);
    assert(result, "windows free memory failed");
}

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static HANDLE stdoutHandle;
static DWORD outModeInit;

void setup_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
    // Enable ANSI escape codes
    DWORD outMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if(stdoutHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }

    if(!GetConsoleMode(stdoutHandle, &outMode)) {
        exit(GetLastError());
    }

    outModeInit = outMode;
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(stdoutHandle, outMode)) {
        exit(GetLastError());
    }
}

void restore_console(void) {
    // https://solarianprogrammer.com/2019/04/08/c-programming-ansi-escape-codes-windows-macos-linux-terminals/
    // Reset console mode
    printf("\x1b[0m");

    if(!SetConsoleMode(stdoutHandle, outModeInit)) {
       exit(GetLastError());
    }
}
