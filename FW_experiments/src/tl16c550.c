#include "tl16c550.h"

// Регистры TL16C550
__sfr __banked __at(0xf8ef) UART_RBR_THR;
__sfr __banked __at(0xf9ef) UART_IER;
__sfr __banked __at(0xfaef) UART_IIR_FCR;
__sfr __banked __at(0xfbef) UART_LCR;
__sfr __banked __at(0xfcef) UART_MCR;
__sfr __banked __at(0xfdef) UART_LSR;
__sfr __banked __at(0xfeef) UART_MSR;
__sfr __banked __at(0xffef) UART_SCR;

// Самодельный stdarg.h
typedef unsigned char *va_list;
#define va_start(ap, last) (ap = ((va_list)&(last)) + sizeof(last))
#define va_arg(ap, type)   (*(type *)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap)         ((void)0)

// Инициализация UART
void uart_init(void) {
    // 1. Сброс FIFO и включение DLAB
    UART_LCR = 0x80;    // DLAB=1
    
    // 2. Установка скорости (115200)
    UART_RBR_THR = 0x01;
    UART_IER = 0x00;
    
    // 3. Формат кадра: 8N1
    UART_LCR = 0x03;     // DLAB=0
    
    // 4. Включаем FIFO
    UART_IIR_FCR = 0x07; // Enable FIFO, clear buffers (00000111b)
    
    // 5. Включаем аппаратный контроль потока
    UART_MCR = 0x2F;     // RTS=1, AFE=1
    
    // 6. Задержка для стабилизации
    for (volatile int i = 0; i < 1000; i++);
}

// Функция отправки символа
void putchar(char c) {
    while ((UART_LSR & 0x20) == 0) {
        __asm
        nop
        __endasm;
    }
    UART_RBR_THR = c;
    // TODO: Добавлено, чтобы убрать "задубливание символов"
    for (volatile int i = 0; i < 42; i++);
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