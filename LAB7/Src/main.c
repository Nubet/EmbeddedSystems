#include "my_hal.h"
#include "lpuart.h"
#include "tim6.h"
#include "fifo.h"
#include "seven_segment_display.h"

extern uint32_t SystemCoreClock;
void SystemInit(void);

static FIFO IncomingDigitsQueue;
static FIFO OutgoingFeedbackQueue;

static volatile char CurrentDisplayChar = ' ';

typedef enum {
	RX_FEEDBACK_OK = 0, RX_FEEDBACK_FIFO_FULL, RX_FEEDBACK_INVALID_CHAR
} RxFeedbackStatus;

static bool IsDigit(char character) {
	return (character >= '0') && (character <= '9');
}

static RxFeedbackStatus EvaluateReceivedCharacter(char received_char) {
	if (!IsDigit(received_char)) {
		return RX_FEEDBACK_INVALID_CHAR;
	}

	if (FIFO_Put(&IncomingDigitsQueue, received_char) == FIFO_OK) {
		return RX_FEEDBACK_OK;
	}

	return RX_FEEDBACK_FIFO_FULL;
}

static char GetFeedbackCharacter(char received_char,
		RxFeedbackStatus feedback_status) {
	if (feedback_status == RX_FEEDBACK_OK) {
		return received_char;
	}
	if (feedback_status == RX_FEEDBACK_FIFO_FULL) {
		return 'x';
	}
	return '.';
}

static void EnqueueFeedbackCharacter(char feedback_char) {
	if (FIFO_Put(&OutgoingFeedbackQueue, feedback_char) == FIFO_OK) {
		LPUART_EnableTxEmptyInterrupt();
	}
}

static void HandleReceivedByte(uint8_t received_byte) {
	char received_char = (char) received_byte;
	RxFeedbackStatus feedback_status = EvaluateReceivedCharacter(received_char);
	char feedback_char = GetFeedbackCharacter(received_char, feedback_status);

	EnqueueFeedbackCharacter(feedback_char);
}

static void HandleTransmitReady(void) {
	char tx_char = 0;

	if (FIFO_Get(&OutgoingFeedbackQueue, &tx_char) != FIFO_OK) {
		LPUART_DisableTxEmptyInterrupt();
		return;
	}

	if (LPUART_TryWriteByte((uint8_t) tx_char) != 0) {
		FIFO_Put(&OutgoingFeedbackQueue, tx_char);
	}
}

static void UpdateSingleDigit(void) {
	char next_digit = ' ';
	if (FIFO_Get(&IncomingDigitsQueue, &next_digit) != FIFO_OK) {
		next_digit = ' ';
	}
	CurrentDisplayChar = next_digit;
}

void LPUART1_IRQHandler(void) {
	if (LPUART_HasAnyError()) {
		LPUART_ClearAllErrors();
	}

	if (LPUART_IsRxNotEmpty()) {
		uint8_t received_byte = 0U;

		if (LPUART_TryReadByte(&received_byte) == 0) {
			HandleReceivedByte(received_byte);
		}
	}

	if (LPUART_IsTxEmptyInterruptEnabled() && LPUART_IsTxEmptyFlagSet()) {
		HandleTransmitReady();
	}
}

void TIM6_DACUNDER_IRQHandler(void) {
	if (TIM6_CheckAndClearUpdateFlag()) {
		UpdateSingleDigit();
	}
}

static void Hardware_Init(void) {
	SystemInit();

	FIFO_Init(&IncomingDigitsQueue);
	FIFO_Init(&OutgoingFeedbackQueue);
	Display_Init();

	LPUART_init();
	LPUART_EnableInterrupts();
	LPUART_DisableTxEmptyInterrupt();

	Activate_Clock_For(&RCC_APB1ENR1, EN_TIM6);
	TIM6_Init(3999, 999);
	TIM6_EnableInterrupts();
}

int main(void) {
	Hardware_Init();

	LPUART_SendString((unsigned char*) "Enter the Numbers\r\n");

	while (1) {
		Display_RenderCurrentState(CurrentDisplayChar);
	}
}
