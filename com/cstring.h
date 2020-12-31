#ifndef CSTRING_H
#define CSTRING_H

int   core_memcmp(const void *buf1, const void *buf2,int n);
void* core_memcpy(void *buf1, const void *buf2, int n);
void* core_memset(void *buf, int ch, int n);
void* core_memmove( void* dest, const void* src, int n );
void  core_memcpyX4(u32* dst,u32* src,int size);
void  core_memsetX4(u32* dst,u32  clr,int size);

int   core_strlen(const char *s);
char* core_strcpy(char *dest, const char *src);
char *core_strchr(const char *s, int c);
char* core_strrchr(const char *src, int c);
char* core_strcat(char *dest, const char *src);
int   core_strcmp(const char *str1, const char *str2);
int   core_strncmp(const char *s1, const char* s2, int n);
int   core_stricmp(const char *str1, const char *str2);
void  core_strrev(char *s);

void  core_itoa(int val, char *s);
void  core_ustoa(unsigned short val, char *s);
void  core_ultoa(unsigned long val, char *s);
unsigned int core_atoh(const unsigned char *pszStr);
int   core_atoi(const char *s);



int   core_stricmp(const char *str1, const char *str2);


#endif
