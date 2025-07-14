// Простая реализация memcpy
void *memcpy(void *dest, const void *src, unsigned int n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    // Копируем побайтно
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

// Аналог memset - заполнение памяти
void *memset(void *ptr, int value, int num) {
    unsigned char *p = (unsigned char *)ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}

// Реализация memmove с поддержкой перекрывающихся областей
void *memmove(void *dest, const void *src, int n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    // Если области не перекрываются или dest < src, копируем вперед
    if (d < s || d >= s + n) {
        while (n--) {
            *d++ = *s++;
        }
    } 
    // Если dest > src и области перекрываются, копируем назад
    else {
        d += n - 1;
        s += n - 1;
        while (n--) {
            *d-- = *s--;
        }
    }
    
    return dest;
}

// Реализация strlen
int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Реализация strcmp
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

#include "arg.h"

// Простая реализация sscanf для %d, %x, %s, %c
int sscanf(const char *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    int count = 0;
    const char *p = fmt;
    
    while (*p && *str) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd': {
                    int *val = va_arg(args, int*);
                    *val = 0;
                    int sign = 1;
                    
                    if (*str == '-') {
                        sign = -1;
                        str++;
                    }
                    
                    while (*str >= '0' && *str <= '9') {
                        *val = *val * 10 + (*str - '0');
                        str++;
                    }
                    *val *= sign;
                    count++;
                    break;
                }
                case 'x': {
                    unsigned int *val = va_arg(args, unsigned int*);
                    *val = 0;
                    
                    while (1) {
                        if (*str >= '0' && *str <= '9') {
                            *val = *val * 16 + (*str - '0');
                        } else if (*str >= 'a' && *str <= 'f') {
                            *val = *val * 16 + (*str - 'a' + 10);
                        } else if (*str >= 'A' && *str <= 'F') {
                            *val = *val * 16 + (*str - 'A' + 10);
                        } else {
                            break;
                        }
                        str++;
                    }
                    count++;
                    break;
                }
                case 's': {
                    char *val = va_arg(args, char*);
                    while (*str && *str != ' ' && *str != '\t') {
                        *val++ = *str++;
                    }
                    *val = '\0';
                    count++;
                    break;
                }
                case 'c': {
                    char *val = va_arg(args, char*);
                    *val = *str++;
                    count++;
                    break;
                }
            }
            p++;
        } else {
            // Пропускаем пробельные символы в формате
            while (*str == ' ' || *str == '\t') str++;
            
            // Сравниваем обычные символы
            if (*p != *str) break;
            
            p++;
            str++;
        }
    }
    
    va_end(args);
    return count;
}
