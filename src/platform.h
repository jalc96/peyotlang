internal void *allocate_memory(memory_size size);
internal void free_memory(void *memory);

#if WIN32
#include"win32_platform.h"
#else
#include"linux_platform.h"
#endif
