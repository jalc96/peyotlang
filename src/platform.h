internal void *allocate_memory(memory_size size);
internal void free_memory(void *memory);
internal void setup_console(void);
internal void restore_console(void);

#if WIN32
#include"win32_platform.h"
#else
#include"linux_platform.h"
#endif
