#include "tl16c550.h"

__sfr __banked __at(0xf8ef) UART_RBR_THR;
__sfr __banked __at(0xf9ef) UART_IER;
__sfr __banked __at(0xfaef) UART_IIR_FCR;
__sfr __banked __at(0xfbef) UART_LCR;
__sfr __banked __at(0xfcef) UART_MCR;
__sfr __banked __at(0xfdef) UART_LSR;
__sfr __banked __at(0xfeef) UART_MSR;
__sfr __banked __at(0xffef) UART_SCR;

void uart_init(void) {
    UART_MCR        = 0x0d;
    UART_IIR_FCR    = 0x87;
    UART_LCR        = 0x83;
    UART_RBR_THR    = 0x01;
    UART_IER        = 0x00;
    UART_LCR        = 0x03;
    UART_IER        = 0x00;
    UART_MCR        = 0x2f;
}

void putchar(char c) {
    UART_RBR_THR = c;
    while ((UART_LSR & 0x20) == 0) {
        __asm
        nop
        __endasm;
    }
    // for (volatile int i = 0; i < 100; i++);
}

int getchar(char *ch) {
    unsigned int timeout = 10;
    while (timeout-- > 0) {
        if (UART_LSR & 0x01) {
            *ch = UART_RBR_THR;
            return 0;
        }
        for (volatile int i = 0; i < 10; i++);
    }
    return -1;
}
