// 15.04.2024 Михаил Каа

#include "main.h"
// #include "font.h"
#include "tl16c550.h"
#include "printf.h"
#include "ucmd.h"
#include "memory_man.h"
#include "z80_ports.h"
#include "defines.h"

#define SCREEN_START_ADR (0x4000)
#define SCREEN_SIZE ((256/8)*192)
#define SCREEN_ATR_SIZE (768)

void init_screen(void);

char *screen = 0x4000;
char w = 0;
char i = 0;
char key[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
            irq_0x38_flag = 0;
            *(screen + 0) = i++;
            port_0xfffd = 0x00;
            port_0xbffd = i;
        }

        if(nmi_0x66_flag) {
            nmi_0x66_flag = 0;
            *(screen + 2) = i;
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

volatile void irq_0x38(void) {
    irq_0x38_flag = 1;

    key[0] = port_0x7ffe;
    key[1] = port_0xeffe;
    key[2] = port_0xbffe;
    key[3] = port_0xdffe;
    key[4] = port_0xf7fe;
    key[5] = port_0xfefe;
    key[6] = port_0xfbfe;
    key[7] = port_0xfdfe;

}

volatile void nmi_0x66(void) {
    nmi_0x66_flag = 1;
}
