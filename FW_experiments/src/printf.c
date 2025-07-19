
#include "arg.h"
#include "tl16c550.h"
#include "printf_zx_scr.h"
#include "string_custom.h"

// Тип функции для вывода символа с контекстом
typedef void (*putchar_func_t)(char c, void* ctx);

// Ядро форматированного вывода
static void vprintf_core(putchar_func_t putchar_func, void* ctx, const char *fmt, va_list args) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '\0') {
                putchar_func('%', ctx);
                break;
            }
            
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            switch (*fmt) {
                case 'd': {
                    int num = va_arg(args, int);
                    char buffer[7];
                    int i = 0;
                    int is_negative = 0;
                    unsigned int abs_num;

                    if (num < 0) {
                        is_negative = 1;
                        abs_num = (unsigned int)(-num);
                    } else {
                        abs_num = (unsigned int)num;
                    }

                    do {
                        buffer[i++] = '0' + (abs_num % 10);
                        abs_num /= 10;
                    } while (abs_num > 0 && i < 6);

                    if (is_negative) {
                        buffer[i++] = '-';
                    }

                    while (i > 0) {
                        putchar_func(buffer[--i], ctx);
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[6]; // 65535 - 5 цифр + null
                    int i = 0;
                    
                    // Преобразуем число в строку (в обратном порядке)
                    do {
                        buffer[i++] = '0' + (num % 10);
                        num /= 10;
                    } while (num > 0 && i < 5);
                    
                    // Выводим символы в правильном порядке
                    while (i > 0) {
                        putchar_func(buffer[--i], ctx);
                    }
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, char*);
                    while (*s) {
                        putchar_func(*s++, ctx);
                    }
                    break;
                }
                case 'c': 
                    putchar_func(va_arg(args, int), ctx);
                    break;
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    const char hex_chars[] = "0123456789abcdef";
                    char buffer[5];
                    int i = 0;
                    int max_chars = 4;

                    if (width > max_chars) width = max_chars;
                    do {
                        buffer[i++] = hex_chars[num & 0x0F];
                        num >>= 4;
                    } while (num > 0 && i < max_chars);
                    while (i < width) buffer[i++] = '0';
                    while (i > 0) putchar_func(buffer[--i], ctx);
                    break;
                }
                default: 
                    putchar_func(*fmt, ctx);
                    break;
            }
        } else {
            putchar_func(*fmt, ctx);
        }
        fmt++;
    }
}

// Обертка для вывода в UART
static void uart_putchar_wrapper(char c, void* ctx) {
    (void)ctx;
    putchar(c);
    // putchar_zx_scr(c);
}

// Обертка для вывода в ZX SCREEN
static void zxscr_putchar_wrapper(char c, void* ctx) {
    (void)ctx;
    // if(IS_CONTROL_CHAR(c))putchar_zx_scr(c);
    // putchar_zx_scr(c);
    terminal_putchar(c);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf_core(uart_putchar_wrapper, NULL, fmt, args);
    va_end(args);
}

void fprintf(int file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if(file == 0 || file == 1 ||file == 2 ){
        vprintf_core(uart_putchar_wrapper, NULL, fmt, args);
    } else if(file == 4) {
        vprintf_core(zxscr_putchar_wrapper, NULL, fmt, args);
    }

    va_end(args);
}

// Структура для контекста буфера
typedef struct {
    char *buffer;
    char *position;
    int max_size;
    int written;
} BufferContext;

// Обертка для вывода в буфер
static void buffer_putchar_wrapper(char c, void* ctx) {
    BufferContext *bc = (BufferContext*)ctx;
    if (bc->written + 1 < bc->max_size) {
        *(bc->position)++ = c;
        bc->written++;
    }
}

// vsnprintf с защитой
static void vsnprintf(char *buffer, int size, const char *fmt, va_list args) {
    if (size == 0) return;
    
    BufferContext ctx = {
        .buffer = buffer,
        .position = buffer,
        .max_size = size,
        .written = 0
    };
    
    vprintf_core(buffer_putchar_wrapper, &ctx, fmt, args);
    *ctx.position = '\0';
}

#define SIZE_MAX 128

void snprintf(char *buffer, int size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);
}

void sprintf(char *buffer, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, SIZE_MAX, fmt, args);
    va_end(args);
}