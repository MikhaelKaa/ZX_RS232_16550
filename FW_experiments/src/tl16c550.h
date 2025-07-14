
#ifndef __TL16C550__
#define __TL16C550__
#include "defines.h"

void uart_init(void);

void putchar(char c);
int getchar(char*);

void print_str(const char *s);
void print_int(int num);

void printf(const char *fmt, ...);
void sprintf(char *buffer, const char *fmt, ...);
void snprintf(char *buffer, int size, const char *fmt, ...);

#endif /* __TL16C550__ */
