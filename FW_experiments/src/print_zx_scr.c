#include "main.h"
#include "font.h"

char x = 0;
char y = 0;
char new_page = 0; // Флаг новой страницы

// Функция для очистки экрана
void clear_screen() {
    unsigned char *screen = (unsigned char*)0x4000;
    for (int i = 0; i < 6144; i++) {
        screen[i] = 0;
    }
}

void putchar_zx_scr(char c) {
    // Очистка экрана перед выводом на новой странице
    if (new_page) {
        clear_screen();
        new_page = 0;
    }
    
    // Обработка специальных символов
    if (c == '\n') {
        x = 0;
        if (++y >= 24) {
            y = 0;
            new_page = 1; // Установка флага новой страницы
        }
        return;
    }
    
    if (c == '\r') {
        x = 0;
        return;
    }
    
    // Пропускаем нулевые символы
    if (c == 0) return;
    
    // Вычисляем адрес в видеопамяти
    unsigned char *screen_addr = (unsigned char*)(0x4000 | 
        ((y & 0x18) << 8) | 
        ((y & 7) << 5) | 
        x);
    
    // Вычисляем адрес глифа в шрифте
    unsigned char *glyph_addr = font_data + (c - 32) * 8;
    
    // Копируем 8 байт глифа на экран
    for (int i = 0; i < 8; i++) {
        *screen_addr = *glyph_addr++;
        screen_addr += 256; // Переход к следующей строке пикселей
    }
    
    // Обновляем координаты с проверкой границ
    if (++x >= 32) {
        x = 0;
        if (++y >= 24) {
            y = 0;
            new_page = 1; // Установка флага новой страницы
        }
    }
}


// TODO: asm реализация печати в экран, рабочая. 
// Оставлена на будующее, когда настанет момент оптимизиции Си кода.
// x & y координаты знакомест
// _font_data шрифт который начинается с символа '!' (33)
// void print(char x, char y, char* text) __naked {
//     (void)(x);
//     (void)(y);
//     (void)(text);
    
//     __asm
//     ld iy, #2
//     add iy, sp
//     ld d, 0(iy)   ;x
//     ld e, 1(iy)   ;y
//     ld l, 2(iy)
//     ld h, 3(iy)
// print_string:
//     //get_adr_on_screen
//     ld a,d
//     and #7
//     rra
//     rra
//     rra
//     rra
//     or e
//     ld e,a
//     ld a,d
//     and #24
//     or #64
//     ld d,a
// print_string_loop:
//     ld a, (hl)
//     and a
//     ret z
//     push hl
//     ld h, #0
//     ld l, a
//     add hl, hl
//     add hl, hl
//     add hl, hl
//     ld bc, #_font_data - #256 ;
//     add hl, bc
//     push de
//     call print_char
//     pop de
//     pop hl
//     inc hl
//     inc de
//     jp print_string_loop
//     ret

// print_char:
//     ld b, #8
// pchar_loop:
//     ld a, (hl)
//     ld (de), a
//     inc d
//     inc hl
//     djnz pchar_loop
//     ret
//     __endasm;
// }