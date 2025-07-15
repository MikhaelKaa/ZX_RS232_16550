#ifndef __STRING_H__
#define __STRING_H__

// Простая реализация memcpy
void *memcpy(void *dest, const void *src, unsigned int n);

// Аналог memset - заполнение памяти
void *memset(void *ptr, int value, int num);

void *memmove(void *dest, const void *src, int n);

// Реализация strlen
int strlen(const char *str);

// Реализация strcmp
int strcmp(const char *s1, const char *s2);

int sscanf(const char *str, const char *fmt, ...);

// Определим недостающие типы
typedef unsigned int size_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;


// 
#ifndef INT_MIN
#define INT_MIN (-32768)  // Для 16-битных систем (Z80)
#endif

#define IS_CONTROL_CHAR(x) ((x)<=31)

#endif // __STRING_H__


