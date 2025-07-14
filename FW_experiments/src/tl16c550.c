#include "tl16c550.h"
#include "arg.h" // like stdarh.h

// Регистры TL16C550
__sfr __banked __at(0xf8ef) UART_RBR_THR;
__sfr __banked __at(0xf9ef) UART_IER;
__sfr __banked __at(0xfaef) UART_IIR_FCR;
__sfr __banked __at(0xfbef) UART_LCR;
__sfr __banked __at(0xfcef) UART_MCR;
__sfr __banked __at(0xfdef) UART_LSR;
__sfr __banked __at(0xfeef) UART_MSR;
__sfr __banked __at(0xffef) UART_SCR;


// Инициализация UART
void uart_init(void) {
    UART_MCR        = 0x0d; // Assert RTS                           64751 13
    UART_IIR_FCR    = 0x87; // Enable fifo 8 level, and clear it    64239 135
    UART_LCR        = 0x83; // 8n1, DLAB=1                          64495 131
    UART_RBR_THR    = 0x01; // 115200 (divider 1)                   63727 1
    UART_IER        = 0x00; // (divider 0).                         63983 0
    UART_LCR        = 0x03; // 8n1, DLAB=0                          64495 3
    UART_IER        = 0x00; // Disable int                          63983 0
    UART_MCR        = 0x2f; // Enable AFE                           64751 47
}

// Функция отправки символа
void putchar(char c) {
    UART_RBR_THR = c;
    while ((UART_LSR & 0x20) == 0) {
        __asm
        nop
        __endasm;
    }
    // TODO: Добавлено, чтобы убрать "задубливание символов"
    for (volatile int i = 0; i < 100; i++);
}

int getchar(char * ch) {
    unsigned int timeout = 10;
    while (timeout-- > 0) {
        if (UART_LSR & 0x01) {
            *ch = UART_RBR_THR;
            return 0;
        }
        // Короткая задержка
        for (volatile int i = 0; i < 10; i++);
    }
    return -1; // Таймаут
}

// Вывод строки
void print_str(const char *s) {
    while (*s) putchar(*s++);
}

// Вывод числа
void print_int(int num) {
    char buffer[10];
    int i = 0;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Преобразуем число в строку (в обратном порядке)
    do {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0 && i < 9);

    // Добавляем знак минус при необходимости
    if (is_negative) {
        buffer[i++] = '-';
    }

    // Выводим символы в правильном порядке
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

// Вывод числа в шестнадцатеричном формате
void print_hex(unsigned int num, int width) {
    const char hex_chars[] = "0123456789abcdef";
    char buffer[10];
    int i = 0;
    
    // Преобразуем число в hex
    do {
        buffer[i++] = hex_chars[num & 0x0F];
        num >>= 4;
    } while (num > 0 && i < (int)sizeof(buffer) - 1);
    
    // Дополняем ведущими нулями
    while (i < width) {
        buffer[i++] = '0';
    }
    
    // Выводим в обратном порядке
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

// Реализация printf
void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            
            // Обработка ширины вывода
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            switch (*fmt) {
                case 'd': 
                    print_int(va_arg(args, int)); 
                    break;
                case 's': 
                    print_str(va_arg(args, char*)); 
                    break;
                case 'c': 
                    putchar(va_arg(args, int)); 
                    break;
                case 'x': 
                    print_hex(va_arg(args, unsigned int), width); 
                    break;
                default: 
                    putchar(*fmt); 
                    break;
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

// TODO: Исправить дублирование кода
#define SIZE_MAX 128

// Структура для контекста буфера с защитой
typedef struct {
    char *buffer;      // Начало буфера
    char *position;    // Текущая позиция записи
    int max_size;   // Максимальный размер буфера (включая нуль-терминатор)
    int written;    // Количество записанных байт (без терминатора)
} BufferContext;

// Защищенная запись символа в буфер
static void buffer_putchar_safe(char c, BufferContext *ctx) {
    // Проверяем, есть ли место для символа и нуль-терминатора
    if (ctx->written + 1 < ctx->max_size) {
        *(ctx->position)++ = c;
        ctx->written++;
    }
}

// Защищенные версии функций вывода
static void print_str_to_buffer_safe(const char *s, BufferContext *ctx) {
    while (*s && ctx->written + 1 < ctx->max_size) {
        *(ctx->position)++ = *s++;
        ctx->written++;
    }
}

static void print_int_to_buffer_safe(int num, BufferContext *ctx) {
    char local_buf[10];
    int i = 0;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    do {
        local_buf[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0 && i < 9);

    if (is_negative) {
        local_buf[i++] = '-';
    }

    while (i > 0 && ctx->written + 1 < ctx->max_size) {
        buffer_putchar_safe(local_buf[--i], ctx);
    }
}

static void print_hex_to_buffer_safe(unsigned int num, int width, BufferContext *ctx) {
    const char hex_chars[] = "0123456789ABCDEF";
    char local_buf[10];
    int i = 0;
    
    do {
        local_buf[i++] = hex_chars[num & 0x0F];
        num >>= 4;
    } while (num > 0 && i < (int)sizeof(local_buf) - 1);
    
    while (i < width && i < (int)sizeof(local_buf)) {
        local_buf[i++] = '0';
    }
    
    while (i > 0 && ctx->written + 1 < ctx->max_size) {
        buffer_putchar_safe(local_buf[--i], ctx);
    }
}

// Основная функция форматированного вывода с защитой
static void vsnprintf(char *buffer, int size, const char *fmt, va_list args) {
    BufferContext ctx = {
        .buffer = buffer,
        .position = buffer,
        .max_size = size,
        .written = 0
    };
    
    // Для пустого буфера сразу завершаем
    if (size == 0) return;
    
    while (*fmt && ctx.written + 1 < ctx.max_size) {
        if (*fmt == '%') {
            fmt++;
            
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            switch (*fmt) {
                case 'd': 
                    print_int_to_buffer_safe(va_arg(args, int), &ctx); 
                    break;
                case 's': 
                    print_str_to_buffer_safe(va_arg(args, char*), &ctx); 
                    break;
                case 'c': 
                    buffer_putchar_safe(va_arg(args, int), &ctx); 
                    break;
                case 'x': 
                    print_hex_to_buffer_safe(va_arg(args, unsigned int), width, &ctx); 
                    break;
                default: 
                    buffer_putchar_safe(*fmt, &ctx); 
                    break;
            }
        } else {
            buffer_putchar_safe(*fmt, &ctx);
        }
        fmt++;
    }
    
    // Гарантированное завершение строки
    *ctx.position = '\0';
}

// Публичные функции
void snprintf(char *buffer, int size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);
}

void sprintf(char *buffer, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Для sprintf используем "достаточно большой" размер
    // Внимание: это не безопасно! Лучше всегда использовать snprintf
    vsnprintf(buffer, SIZE_MAX, fmt, args);
    
    va_end(args);
}