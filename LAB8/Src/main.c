#include <stdint.h>
#include "cli.h"
#include "menu.h"
#include "lpuart.h"
#include "seven_segment_display.h"
#include "led_driver.h"
#include "joystick.h"
#include "tim6.h"

void SystemInit(void);

// Timer clock = 4 MHz / (PSC + 1) = 4 MHz / 4 = 1 MHz
// Update event = 1 MHz / (ARR + 1) = 1 MHz / 1000 = 1 kHz = 1 ms
#define TIM6_PSC  3U
#define TIM6_ARR  999U

volatile uint32_t system_millis = 0U;

void delay_ms(uint32_t ms_to_wait) {
	uint32_t start_time = system_millis;

	while ((system_millis - start_time) < ms_to_wait) {
	}
}

void TIM6_DACUNDER_IRQHandler(void) {
    if (TIM6_CheckAndClearUpdateFlag()) {
		system_millis++;
        Display_RenderCurrentState();
    }
}

static void Hardware_Init(void) {
	SystemInit();
	LPUART_init();
	Display_Init();
	LED_Init();
	JOY_Init();
	Activate_Clock_For(&RCC_APB1ENR1, EN_TIM6);
	TIM6_Init(TIM6_PSC, TIM6_ARR);
	TIM6_EnableInterrupts();
	Menu_Init();
}

int main(void) {
	char line[CLI_LINE_BUFFER_SIZE];

	Hardware_Init();

	CLI_Print(
			"Simple menu by Norbert Fila\r\nType 'help' for more information\r\n");
	CLI_Print(ANSI_CURSOR_SHOW);
	Menu_PrintPromptForCurrentContext();

	while (1) {

		// Skip command execution until a full line is assembled (Enter pressed)
		if (!CLI_CheckForIncomingCharacterAndAssembleLine(line)) {
			continue;
		}

		Menu_ExecuteCommandInCurrentContext(line);
		Menu_PrintPromptForCurrentContext();
	}
}
