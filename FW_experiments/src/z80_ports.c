/**
 * @file z80_ports.c
 * @brief z80 ports utility
 * @author Mikhael Kaa (Михаил Каа)
 * @date 15.07.2025
 */

#include "string_custom.h"
#include "printf.h"
#include "tl16c550.h"
#include "errno.h"
#include "defines.h"

static void print_usage(void);

#define ENDL "\r\n"

// Функция чтения из порта
static uint8_t inport(uint16_t port) __naked {
    (void)(port);
    __asm
        ld hl, #2       
        add hl, sp
        ld c, (hl)     
        inc hl
        ld b, (hl)     
        in a, (c)      
        ld l, a      
        ret
    __endasm;
}

// Функция записи в порт
static void outport(uint16_t port, uint8_t data) __naked {
    (void)(port);
    (void)(data);
    __asm
        ld hl, #2      
        add hl, sp
        ld c, (hl)    
        inc hl
        ld b, (hl)     
        
        ld hl, #4    
        add hl, sp
        ld a, (hl)    
        
        out (c), a 
        ret
    __endasm;
}

int ucmd_port(int argc, char *argv[]) {
    uint16_t port;
    uint16_t data;

    switch (argc) {
    case 1:
    case 2:
        print_usage();
        return -EINVAL;
    
    case 3:
        if (strcmp(argv[1], "read") == 0) {
            if (sscanf(argv[2], "%x", &port) != 1) {
                printf("Invalid port format" ENDL);
                return -EINVAL;
            }
            uint8_t value = inport(port);
            printf("Port 0x%04x: 0x%02x" ENDL, port, value);
            return 0;
        }
        break;
    
    case 4:
        if (strcmp(argv[1], "write") == 0) {
            if (sscanf(argv[2], "%x", &port) != 1 ||
                sscanf(argv[3], "%x", &data) != 1) 
            {
                printf("Invalid arguments format" ENDL);
                return -EINVAL;
            }
            outport(port, (uint8_t)data);
            printf("Wrote 0x%02x to port 0x%04x" ENDL, data, port);
            return 0;
        }
        break;

    default:
        print_usage();
        return -EINVAL;
    }
    
    print_usage();
    return -EINVAL;
}

static void print_usage(void) {
    printf("Usage: port <command> [arguments]" ENDL);
    printf("Commands:" ENDL);
    printf("  read <port>          - Read byte from port" ENDL);
    printf("  write <port> <data>  - Write byte to port" ENDL);
}

#undef ENDL