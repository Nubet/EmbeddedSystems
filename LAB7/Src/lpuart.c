#include "lpuart.h"
#include "my_hal.h"

#define RCC_APB1ENR2 (*(volatile uint32_t *)(RCC_BASE + 0x5C))
#define EN_LPUART1   BIT(0)

#define NVIC_ISER2 (*((volatile uint32_t *)0xE000E108))
#define NVIC_LPUART1_EN BIT(6)

extern uint32_t SystemCoreClock;

static void LPUART_Enable_Clocks(void) {
	Activate_Clock_For(&RCC_AHB2ENR, EN_GPIOC);

	// Select System Clock as LPUART1 clock
	RCC_CCIPR &= ~RCC_CCIPR_LPUART1SEL_Msk;
	RCC_CCIPR |= RCC_CCIPR_LPUART1SEL_SYSCLK;

	Activate_Clock_For(&RCC_APB1ENR2, EN_LPUART1);
}

static void LPUART_Init_Pins(void) {
	GPIO_InitTypeDef lpuart_pins = { 0 };

	lpuart_pins.Mode = MODE_ALTERNATE;
	lpuart_pins.Alternate = LPUART1_AF;
	lpuart_pins.Pull = PUDPR_PULL_UP;
	lpuart_pins.Speed = OSPEEDR_OUTPUT_HIGHSPEED;
	lpuart_pins.Type = OTYPER_TYPE_PP;

	lpuart_pins.Pin = LPUART1_RX_PIN;
	GPIO_Init(LPUART1_PORT, &lpuart_pins);

	lpuart_pins.Pin = LPUART1_TX_PIN;
	GPIO_Init(LPUART1_PORT, &lpuart_pins);
}

static void LPUART_Config_Peripheral(uint32_t target_baudrate) {
	// Disable LPUART (UE=0) before modifying configuration registers
	CLR_BIT(MY_LPUART1->CR1, LPUART_CR1_UE_Pos);

	// Configure 8-bit data length (M1=0, M0=0) for 8N1 mode
	CLR_BIT(MY_LPUART1->CR1, LPUART_CR1_M0_Pos);
	CLR_BIT(MY_LPUART1->CR1, LPUART_CR1_M1_Pos);

	// Disable parity control (PCE=0) for 8N1 mode
	CLR_BIT(MY_LPUART1->CR1, LPUART_CR1_PCE_Pos);

	// Configure 1 Stop bit (STOP=00 in CR2) for 8N1 mode
	CLR_BIT(MY_LPUART1->CR2, LPUART_CR2_STOP_B0_Pos);
	CLR_BIT(MY_LPUART1->CR2, LPUART_CR2_STOP_B1_Pos);

	// Disable DMA requests (DMAT=0, DMAR=0) to force standard CPU polling mode
	CLR_BIT(MY_LPUART1->CR3, LPUART_CR3_DMAR_Pos);
	CLR_BIT(MY_LPUART1->CR3, LPUART_CR3_DMAT_Pos);

	// Set baud rate using: (256 * fCK) / Baudrate
	MY_LPUART1->BRR = (256UL * SystemCoreClock) / target_baudrate;

	// Enable LPUART (UE=1), Transmitter (TE=1) and Receiver (RE=1) after full setup
	SET_BIT(MY_LPUART1->CR1, LPUART_CR1_UE_Pos);
	SET_BIT(MY_LPUART1->CR1, LPUART_CR1_TE_Pos);
	SET_BIT(MY_LPUART1->CR1, LPUART_CR1_RE_Pos);
}

int LPUART_init(void) {
	LPUART_Enable_Clocks();
	LPUART_Init_Pins();
	LPUART_Config_Peripheral(LPUART_BAUDRATE);
	return 0;
}

int LPUART_SendChar(unsigned char data) {
	while (Read_Specific_32Register_Bit(&(MY_LPUART1->ISR), LPUART_ISR_TXE_Pos)
			== 0) {
	}
	MY_LPUART1->TDR = data;
	return 0;
}

int LPUART_SendString(unsigned char *data) {
	while (*data) {
		LPUART_SendChar(*data++);
	}
	return 0;
}

int LPUART_ReceiveChar(unsigned char *data) {
	if (Read_Specific_32Register_Bit(&(MY_LPUART1->ISR), LPUART_ISR_RXNE_Pos)
			== 1) {
		*data = (unsigned char) (MY_LPUART1->RDR & LPUART_ASCII_CHAR_Msk);
		return 0;
	}
	return -1;
}

int LPUART_TryWriteByte(uint8_t data) {
	if (Read_Specific_32Register_Bit(&(MY_LPUART1->ISR), LPUART_ISR_TXE_Pos)
			== 0) {
		return -1;
	}

	MY_LPUART1->TDR = data;
	return 0;
}

int LPUART_TryReadByte(uint8_t *data) {
	if (Read_Specific_32Register_Bit(&(MY_LPUART1->ISR), LPUART_ISR_RXNE_Pos)
			== 0) {
		return -1;
	}

	*data = (uint8_t) (MY_LPUART1->RDR & LPUART_ASCII_CHAR_Msk);
	return 0;
}

static inline void NVIC_EnableLpuart1Interrupt(void) {
	NVIC_ISER2 |= NVIC_LPUART1_EN;
}
void LPUART_EnableInterrupts(void) {
	LPUART_EnableRxInterrupt();
	NVIC_EnableLpuart1Interrupt();
}

void LPUART_EnableRxInterrupt(void) {
	SET_BIT(MY_LPUART1->CR1, LPUART_CR1_RXNEIE_Pos);
}

void LPUART_EnableTxEmptyInterrupt(void) {
	SET_BIT(MY_LPUART1->CR1, LPUART_CR1_TXEIE_Pos);
}

void LPUART_DisableTxEmptyInterrupt(void) {
	CLR_BIT(MY_LPUART1->CR1, LPUART_CR1_TXEIE_Pos);
}
