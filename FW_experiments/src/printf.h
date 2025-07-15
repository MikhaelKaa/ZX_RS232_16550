
#ifndef __PRINTF__
#define __PRINTF__


void print_str(const char *s);
void print_int(int num);

void printf(const char *fmt, ...);
void sprintf(char *buffer, const char *fmt, ...);
void snprintf(char *buffer, int size, const char *fmt, ...);
void fprintf(int file, const char *fmt, ...);

#endif /* __PRINTF__ */
