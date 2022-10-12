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
