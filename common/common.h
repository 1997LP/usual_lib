#ifndef __COMMON__
#define __COMMON__

#include <stddef.h>
#include "printf.h"

#ifndef __HAVE_ARCH_STRCPY

#undef strcmp
#undef strncpy
#undef strcat
#undef strncat
#undef strcmp
#undef strncmp
#undef strlen
#undef strnlen
#undef memset
#undef memcpy

extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern char * strcat(char *, const char *);
extern char * strncat(char *, const char *, size_t);
extern int  strcmp(const char *,const char *);
extern int  strncmp(const char *,const char *,size_t);
extern size_t strlen(const char *);
extern size_t strnlen(const char *,size_t);
extern void * memset(void *,int,size_t);
extern void * memcpy(void *,const void *,size_t);
extern int  memcmp(const void *,const void *,size_t);

extern void cmdLoop(void);
extern int  xmodem_download(unsigned int);
extern void memDisp(void *, unsigned int);
extern char HexToChar(unsigned char );
extern void ByteToStr(unsigned char , char *);
extern void ShortToStr(unsigned short , char *);
extern void IntToStr(unsigned int , char *);
#endif

#endif
