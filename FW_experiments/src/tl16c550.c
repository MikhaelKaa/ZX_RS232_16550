
#include "tl16c550.h"

void delay(unsigned int t) {
    for(unsigned int j = 0; j != t; j++) {
        __asm
        nop
        __endasm;
    }
}

char uart_get(char* data) {
    if(port_0xfdef & 1) {
        *data = port_0xf8ef;
        return 0;
    }
    return -1;
}

// tl16c550 init.
void uart_init(void) {
    port_0xfcef = 0x0d; // Assert RTS                           64751 13
    port_0xfaef = 0x87; // Enable fifo 8 level, and clear it    64239 135
    port_0xfbef = 0x83; // 8n1, DLAB=1                          64495 131
    port_0xf8ef = 0x01; // 115200 (divider 1)                   63727 1
    port_0xf9ef = 0x00; // (divider 0).                         63983 0
    port_0xfbef = 0x03; // 8n1, DLAB=0                          64495 3
    port_0xf9ef = 0x00; // Disable int                          63983 0
    port_0xfcef = 0x2f; // Enable AFE                           64751 47
}

void uart_print(char* text) {
    char ii = 0;
    while(*(text+ii) != 0) {
        port_0xf8ef = *(text+ii++); // 63727
        delay(10);
    }
}
