#include "my_hal.h"
#include "lpuart.h"

extern uint32_t SystemCoreClock;
void SystemInit(void);

static void Hardware_Init(void) {
    SystemInit();
    LPUART_init();
}

static unsigned char Toggle_Case(unsigned char c) {

    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    else if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

int main(void) {
    Hardware_Init();

    LPUART_SendString((unsigned char*)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n");

    LPUART_SendString((unsigned char*)"Welcome message Norbert\r\n");

    while (1) {
        unsigned char received_data;

        if (LPUART_ReceiveChar(&received_data) == 0) {
            unsigned char processed_data = Toggle_Case(received_data);
            LPUART_SendChar(processed_data);
        }

    }
}
