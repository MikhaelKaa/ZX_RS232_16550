#include "main.h"
#include "font.h"
#include "string_custom.h"

// Глобальные переменные терминала
static unsigned char term_x = 0;
static unsigned char term_y = 23;  // Начинаем с последней строки

// Состояния парсера ESC-последовательностей
typedef enum {
    STATE_NORMAL,
    STATE_ESCAPE,
    STATE_CSI
} parser_state_t;

static parser_state_t parser_state = STATE_NORMAL;
static char esc_buffer[16];
static unsigned char esc_index = 0;

// Функция очистки экрана
void clear_screen() {
    // Очищаем пиксельные данные
    unsigned char *screen = (unsigned char*)0x4000;
    unsigned int i;
    for (i = 0; i < 6144; i++) {
        screen[i] = 0;
    }
    
    // Очищаем атрибуты
    unsigned char *attrs = (unsigned char*)0x5800;
    for (i = 0; i < 768; i++) {
        attrs[i] = 0;
    }
    
    term_x = 0;
    term_y = 23;
}

// Прокрутка экрана вверх на одну строку
void scroll_screen() {
    unsigned char line, col, scan;
    unsigned int src_addr, dst_addr;
    unsigned char *src, *dst;
    
    // Копируем строки пикселей
    for (line = 0; line < 23; line++) {
        for (col = 0; col < 32; col++) {
            // Адрес источника (строка ниже)
            src_addr = 0x4000;
            src_addr |= ((line + 1) & 0x18) << 8;
            src_addr |= ((line + 1) & 7) << 5;
            src_addr |= col;
            src = (unsigned char*)src_addr;
            
            // Адрес назначения (текущая строка)
            dst_addr = 0x4000;
            dst_addr |= (line & 0x18) << 8;
            dst_addr |= (line & 7) << 5;
            dst_addr |= col;
            dst = (unsigned char*)dst_addr;
            
            // Копируем 8 строк пикселей
            for (scan = 0; scan < 8; scan++) {
                *dst = *src;
                src += 256;
                dst += 256;
            }
        }
    }
    
    // Копируем атрибуты строк
    unsigned char *attr_src = (unsigned char*)0x5800 + 32;
    unsigned char *attr_dst = (unsigned char*)0x5800;
    for (int i = 0; i < 23 * 32; i++) {
        *attr_dst++ = *attr_src++;
    }
    
    // Очищаем последнюю строку (пиксели)
    unsigned int last_line_addr = 0x4000;
    last_line_addr |= (23 & 0x18) << 8;
    last_line_addr |= (23 & 7) << 5;
    
    unsigned char *last_line = (unsigned char*)last_line_addr;
    for (col = 0; col < 32; col++) {
        unsigned char *pixel_ptr = last_line + col;
        for (scan = 0; scan < 8; scan++) {
            *pixel_ptr = 0;
            pixel_ptr += 256;
        }
    }
    
    // Очищаем атрибуты последней строки
    unsigned char *last_attr = (unsigned char*)0x5800 + 23 * 32;
    for (col = 0; col < 32; col++) {
        *last_attr++ = 0;
    }
    
    term_x = 0;
    term_y = 23;
}

// Установка курсора
void terminal_set_cursor(unsigned char x, unsigned char y) {
    if (x < 32 && y < 24) {
        term_x = x;
        term_y = y;
    }
}

// Обработка ESC-последовательностей
void process_escape_sequence() {
    if (esc_index < 1) return;
    
    // Обработка CSI последовательностей (начинаются с '[')
    if (esc_buffer[0] == '[') {
        unsigned char num1 = 0, num2 = 0;
        unsigned char has_num1 = 0, has_num2 = 0;
        unsigned char i = 1;  // Пропускаем '['
        
        // Парсим первое число
        while (i < esc_index && esc_buffer[i] >= '0' && esc_buffer[i] <= '9') {
            num1 = num1 * 10 + (esc_buffer[i] - '0');
            has_num1 = 1;
            i++;
        }
        
        // Пропускаем разделитель
        if (i < esc_index && esc_buffer[i] == ';') {
            i++;
            
            // Парсим второе число
            while (i < esc_index && esc_buffer[i] >= '0' && esc_buffer[i] <= '9') {
                num2 = num2 * 10 + (esc_buffer[i] - '0');
                has_num2 = 1;
                i++;
            }
        }
        
        // Определяем команду
        if (i < esc_index) {
            char command = esc_buffer[i];
            
            // Очистка экрана: ESC[2J
            if (command == 'J' && has_num1 && num1 == 2) {
                clear_screen();
            }
            // Установка позиции курсора: ESC[<row>;<col>H
            else if (command == 'H') {
                // Значения по умолчанию 1, если не указаны
                if (!has_num1) num1 = 1;
                if (!has_num2) num2 = 1;
                
                // Преобразуем в 0-based координаты
                unsigned char y = (num1 > 0) ? (num1 - 1) : 0;
                unsigned char x = (num2 > 0) ? (num2 - 1) : 0;
                
                // Устанавливаем курсор
                terminal_set_cursor(x, y);
            }
        }
    }
}

// Основная функция вывода символа
void terminal_putchar(char c) {
    // Обработка ESC-последовательностей
    if (parser_state == STATE_ESCAPE) {
        parser_state = STATE_CSI;
        esc_index = 0;
        esc_buffer[esc_index++] = c;
        return;
    }
    else if (parser_state == STATE_CSI) {
        // Завершение последовательности буквой
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || esc_index >= sizeof(esc_buffer) - 1) {
            esc_buffer[esc_index] = '\0';
            process_escape_sequence();
            parser_state = STATE_NORMAL;
        }
        // Продолжение последовательности
        else if (esc_index < sizeof(esc_buffer) - 1) {
            esc_buffer[esc_index++] = c;
        }
        return;
    }

    // Начало ESC-последовательности
    if (c == 0x1B) {
        parser_state = STATE_ESCAPE;
        return;
    }

    // Обработка управляющих символов
    if (c == '\n') {
        term_x = 0;
        term_y++;
        if (term_y > 23) {
            scroll_screen();
        }
        return;
    }
    
    if (c == '\r') {
        term_x = 0;
        return;
    }
    
    if (c == '\b') {
        if (term_x > 0) {
            term_x--;
            // Стираем символ
            unsigned int addr = 0x4000;
            addr |= (term_y & 0x18) << 8;
            addr |= (term_y & 7) << 5;
            addr |= term_x;
            
            unsigned char *screen_addr = (unsigned char*)addr;
            unsigned char i;
            for (i = 0; i < 8; i++) {
                *screen_addr = 0;
                screen_addr += 256;
            }
        }
        return;
    }
    
    // Вывод обычного символа
    unsigned int addr = 0x4000;
    addr |= (term_y & 0x18) << 8;
    addr |= (term_y & 7) << 5;
    addr |= term_x;
    
    unsigned char *screen_addr = (unsigned char*)addr;
    unsigned char *glyph_addr = font_data + (c - 32) * 8;
    unsigned char i;
    
    for (i = 0; i < 8; i++) {
        *screen_addr = *glyph_addr++;
        screen_addr += 256;
    }
    
    // Установка атрибутов (INK 7, PAPER 0)
    unsigned char *attr_addr = (unsigned char*)(0x5800 + term_y * 32 + term_x);
    *attr_addr = 0x07;  // White on Black
    
    // Перемещаем курсор
    term_x++;
    if (term_x > 31) {
        term_x = 0;
        term_y++;
        if (term_y > 23) {
            scroll_screen();
        }
    }
}

// Функция вывода строки
void terminal_puts(const char *str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}

// Функция установки курсора (публичная)
void terminal_set_cursor_public(unsigned char x, unsigned char y) {
    terminal_set_cursor(x, y);
}
