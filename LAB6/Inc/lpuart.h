#ifndef LPUART_H
#define LPUART_H

#include <stdint.h>

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t reserved0;
    volatile uint32_t reserved1;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint16_t RDR;
    volatile uint16_t reserved2;
    volatile uint16_t TDR;
    volatile uint16_t reserved3;
} LPUART_TypeDef;


#define LPUART1_BASE 0x40008000UL
#define MY_LPUART1   ((LPUART_TypeDef *)LPUART1_BASE)


#define LPUART1_PORT       GPIOC
#define LPUART1_RX_PIN     0U
#define LPUART1_TX_PIN     1U
#define LPUART1_AF         8U
#define LPUART_BAUDRATE 115200UL

#define LPUART_CR1_UE_Pos        0U   // USART Enable
#define LPUART_CR1_RE_Pos        2U   // Receiver Enable
#define LPUART_CR1_TE_Pos        3U   // Transmitter Enable
#define LPUART_CR1_PCE_Pos       10U  // Parity Control Enable
#define LPUART_CR1_M0_Pos        12U  // Word Length Bit 0
#define LPUART_CR1_M1_Pos        28U  // Word Length Bit 1

#define LPUART_CR2_STOP_B0_Pos   12U  // STOP Bit configuration (Bit 0)
#define LPUART_CR2_STOP_B1_Pos   13U  // STOP Bit configuration (Bit 1)

#define LPUART_CR3_DMAR_Pos      6U   // DMA Enable Receiver
#define LPUART_CR3_DMAT_Pos      7U   // DMA Enable Transmitter

#define LPUART_ISR_RXNE_Pos      5U   // Read Data Register Not Empty
#define LPUART_ISR_TXE_Pos       7U   // Transmit Data Register Empty

#define LPUART_ASCII_CHAR_Msk     (0xFFUL)


int LPUART_init(void);
int LPUART_SendChar(unsigned char data);
int LPUART_ReceiveChar(unsigned char* data);
int LPUART_SendString(unsigned char* data);

#endif // LPUART_H
