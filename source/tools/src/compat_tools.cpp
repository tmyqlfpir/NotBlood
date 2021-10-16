// Compatibility declarations for the tools to avoid linking to the entire engine.

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// initprintf() -- prints a string
//
int initprintf(const char *f, ...)
{
    va_list va;
    char buf[2048];

    va_start(va, f);
    int len = Bvsnprintf(buf, sizeof(buf), f, va);
    va_end(va);

    return len;
}

void initputs (const char * str) { puts(str); }

int16_t editstatus = 1;

void engineDestroyAllocator(void)
{
    _sm_allocator_thread_cache_destroy(g_sm_heap);
    _sm_allocator_destroy(g_sm_heap);
}

#ifdef __cplusplus
}
#endif
