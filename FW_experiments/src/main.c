// 15.04.2024 Михаил Каа

#include "main.h"
// #include "font.h"
#include "tl16c550.h"
#include "printf.h"
#include "ucmd.h"
#include "memory_man.h"
#include "z80_ports.h"
#include "defines.h"
#include "string_custom.h"

#define SCREEN_START_ADR (0x4000)
#define SCREEN_SIZE ((256/8)*192)
#define SCREEN_ATR_SIZE (768)

void init_screen(void);


char *screen = 0x4000;
char w = 0;
char i = 0;
// Глобальные переменные
volatile uint8_t key_current[8];
volatile uint8_t key_last[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
volatile uint8_t caps_shift = 0;
volatile uint8_t sym_shift = 0;

// Буфер для ASCII символов
#define KEYBUF_SIZE 32
volatile uint8_t keybuf[KEYBUF_SIZE];
volatile uint8_t keybuf_head = 0;
volatile uint8_t keybuf_tail = 0;

// Таблица преобразования клавиш в ASCII
const uint8_t keymap[8][5] = {
    // Row 0: SPACE, SYM, M, N, B (порт 0x7ffe)
    {' ',  0,  'm', 'n', 'b'},
    // Row 1: 0, 9, 8, 7, 6 (порт 0xeffe)
    {'0', '9', '8', '7', '6'},
    // Row 2: ENTER, L, K, J, H (порт 0xbffe)
    {'\n','l', 'k', 'j', 'h'},
    // Row 3: P, O, I, U, Y (порт 0xdffe)
    {'p', 'o', 'i', 'u', 'y'},
    // Row 4: 1, 2, 3, 4, 5 (порт 0xf7fe)
    {'1', '2', '3', '4', '5'},
    // Row 5: CAPS, Z, X, C, V (порт 0xfefe)
    { 0,  'z', 'x', 'c', 'v'},
    // Row 6: Q, W, E, R, T (порт 0xfbfe)
    {'q', 'w', 'e', 'r', 't'},
    // Row 7: A, S, D, F, G (порт 0xfdfe)
    {'a', 's', 'd', 'f', 'g'}
};


// Функция обработки символа (должна быть реализована)
void process_key(uint8_t ascii) {
    // Добавление символа в кольцевой буфер
    uint8_t next_head = (keybuf_head + 1) % KEYBUF_SIZE;
    if(next_head != keybuf_tail) {
        keybuf[keybuf_head] = ascii;
        keybuf_head = next_head;
    }
}

// Преобразование позиции клавиши в ASCII
uint8_t key_to_ascii(uint8_t row, uint8_t bit) {
    uint8_t ch = keymap[row][bit];
    
    if (sym_shift) {
        // Обработка SYM SHIFT для цифр и специальных символов
        switch (ch) {
            case '0': return '_';
            case '1': return '!';
            case '2': return '@';
            case '3': return '#';
            case '4': return '$';
            case '5': return '%';
            case '6': return '&';
            case '7': return '\'';
            case '8': return '(';
            case '9': return ')';
            case 'k': return '+';
            case 'l': return '=';
            case 'm': return '?';
            case 'n': return ',';
            case 'b': return '*';
            case 'h': return '\\';
            case 'j': return '-';
            case 'u': return '<';
            case 'i': return '>';
            case 'o': return '[';
            case 'p': return ']';
            case 'y': return '{';
            case 't': return '}';
            case 'r': return '~';
            case 'e': return '^';
            case 'w': return '|';
            case 'q': return '`';
            case 'a': return ':';
            case 's': return ';';
            case 'd': return '"';
            case 'f': return '/';
            case 'g': return '=';
            case 'v': return '.';
            case 'c': return ',';
            case 'x': return '?';
            case 'z': return '!';
            default: break;
        }
    }
    
    // Обработка CAPS SHIFT для букв
    if (ch >= 'a' && ch <= 'z' && caps_shift) {
        return ch - 32; // Преобразование в верхний регистр
    }
    
    return ch;
}

static volatile char irq_0x38_flag = 0;
static volatile char nmi_0x66_flag = 0;

char msg[] = "Hello world!!!";

int scr(int argc, char *argv[]) {
    for(int i = 0; i < argc; i++){
        fprintf(4, "%s ", argv[i]);
    }
    fprintf(4, "\r\n");
    return 0;
}

int reset(int argc, char *argv[]) {
    (void)(argc);
    (void)(argv);
    
    __asm
    jp 0
    __endasm;
    return -1;
}

// Пример cmd_list.
command_t cmd_list[] = {
  {
    .cmd  = "help",
    .help = "print available commands with their help text",
    .fn   = print_help_cb,
  },
  {
    .cmd  = "mem",
    .help = "mem",
    .fn   = ucmd_mem,
  },
  {
    .cmd  = "port",
    .help = "port",
    .fn   = ucmd_port,
  },
  {
    .cmd  = "scr",
    .help = "scr",
    .fn   = scr,
  },
    {
    .cmd  = "reset",
    .help = "reset",
    .fn   = reset,
  },
  {0} // null list terminator DON'T FORGET THIS!
};

void main() {
    uart_init();
    
    init_screen();
    printf("\r\n\r\n***********************\r\n");
    printf("init 0\r\n");
    printf("Its work!!!\r\n");
    printf("test d %d\r\n", 42);
    printf("test s %s\r\n", msg);
    printf("test c %c\r\n", 'U');
    printf("test s&d %s %d\r\n", msg, 73);
    ucmd_default_init();

    while(1) {
        ucmd_default_proc();

        if(irq_0x38_flag) {
          // irq_0x38_flag = 0;
          // for(int n = 0; n < 8; n++) {
          //   key[n] &= 31;
          //   if(key[n] != key_last[n]) {
          //     printf("key[%d] = %d\r\n", n, key[n]);
          //     key_last[n] = key[n];
          //   }
          // }
        }

        // Чтение из буфера в основном цикле
        if(keybuf_tail != keybuf_head) {
            uint8_t ascii = keybuf[keybuf_tail];
            keybuf_tail = (keybuf_tail + 1) % KEYBUF_SIZE;
            // printf("new char\r\n");
            printf("%c", ascii);
        }

        if(nmi_0x66_flag) {
            nmi_0x66_flag = 0;
            *(screen + 2) = i;
            printf("nmi\r\n");
        }
    }
}

void init_screen(void) {
    port_0x00fe = 7;
    for (unsigned int i = SCREEN_START_ADR; i < (SCREEN_START_ADR+SCREEN_SIZE); i++) {
        *((char *)i) = 0;
    } 
    for (unsigned int i = SCREEN_START_ADR+SCREEN_SIZE; i < (SCREEN_START_ADR+SCREEN_SIZE+SCREEN_ATR_SIZE); i++) {
        *((char *)i) = 4;
    }
    port_0x00fe = 0;
}

// Обработчик прерывания
volatile void irq_0x38(void) {
  irq_0x38_flag = 1;

  // Чтение всех строк клавиатуры
  key_current[0] = port_0x7ffe & 0x1F;
  key_current[1] = port_0xeffe & 0x1F;
  key_current[2] = port_0xbffe & 0x1F;
  key_current[3] = port_0xdffe & 0x1F;
  key_current[4] = port_0xf7fe & 0x1F;
  key_current[5] = port_0xfefe & 0x1F;
  key_current[6] = port_0xfbfe & 0x1F;
  key_current[7] = port_0xfdfe & 0x1F;

  // Обновление модификаторов
  caps_shift = !(key_current[5] & 0x01); // CAPS SHIFT в row5, bit0
  sym_shift = !(key_current[0] & 0x02);  // SYM SHIFT в row0, bit1

  // Обработка изменений клавиш
  for(uint8_t row = 0; row < 8; row++) {
      uint8_t current = key_current[row];
      uint8_t last = key_last[row];
      uint8_t diff = current ^ last;
      
      if(diff) {
          for(uint8_t bit = 0; bit < 5; bit++) {
              uint8_t mask = 1 << bit;
              
              // Проверка изменения бита
              if(diff & mask) {
                  // Обнаружено нажатие (0 - нажата)
                  if(!(current & mask)) {
                      uint8_t ascii = key_to_ascii(row, bit);
                      
                      // Игнорировать специальные клавиши (0 в таблице)
                      if(ascii) {
                          process_key(ascii);
                      }
                  }
              }
          }
          key_last[row] = current;
      }
  }
}

volatile void nmi_0x66(void) {
    nmi_0x66_flag = 1;
}
