#include "cstring.h"

void* memset(void *buf, int ch, int n)
{
    return core_memset(buf,ch,n);
}


void* memcpy(void *buf1, const void *buf2, int n)
{
    return core_memcpy(buf1,buf2,n);
}

void* memmove( void* dest, const void* src, int n )
{
    return core_memmove(dest,src,n);
}

int strlen(const char *s)
{
    return core_strlen(s);
}


