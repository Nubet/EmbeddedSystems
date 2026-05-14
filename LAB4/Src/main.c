#include "my_hal.h"
#include <stdint.h>

extern uint32_t SystemCoreClock;
void SystemInit(void);

#define SEG_A BIT(0)
#define SEG_B BIT(1)
#define SEG_C BIT(2)
#define SEG_D BIT(3)
#define SEG_E BIT(4)
#define SEG_F BIT(5)
#define SEG_G BIT(6)
#define ALL_SEGMENTS_MASK (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)

#define DIG1 BIT(2)
#define DIG2 BIT(3)
#define DIG3 BIT(4)
#define DIG4 BIT(5)
#define ALL_DIGITS_MASK (DIG1 | DIG2 | DIG3 | DIG4)

#define JOY_PIN 15
#define JOY_PORT GPIOE

#define COUNTER_MIN      0
#define COUNTER_MAX      9999
#define COUNTER_STEP_MS  1000
#define MUX_STEP_MS      5

volatile uint32_t system_ticks = 0;
static volatile int32_t current_counter = 0;
static volatile int8_t count_direction = 1;

void SysTick_Handler(void) {
	system_ticks++;
}

// @formatter:off
	static const uint16_t seven_segment_digit_patterns[10] = {
			SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
			SEG_B | SEG_C,
			SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
			SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
			SEG_B | SEG_C | SEG_F | SEG_G,
			SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
			SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
			SEG_A | SEG_B | SEG_C,
			SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
			SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
	};
// @formatter:on

static void SysTick_Init(void) {
	// exact number of clock cycles for 1ms.
	STK_LOAD = (uint32_t)(SystemCoreClock / 1000UL) - 1UL;

	// Reset current counter value
	STK_VAL = 0UL;

	// Enable SysTick: Use processor clock, enable interrupt, start timer
	STK_CTRL = STK_CTRL_CLKSOURCE | STK_CTRL_TICKINT | STK_CTRL_ENABLE;
}
static void Display_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Mode = MODE_OUTPUT;
	GPIO_InitStruct.Pull = PUDPR_PULL_NO;
	GPIO_InitStruct.Speed = OSPEEDR_OUTPUT_LOWSPEED;
	GPIO_InitStruct.Type = OTYPER_TYPE_PP;

	for (uint8_t i = 0; i <= 6; i++) {
		GPIO_InitStruct.Pin = i;
		GPIO_Init(GPIOG, &GPIO_InitStruct);
	}

	for (uint8_t i = 2; i <= 5; i++) {
		GPIO_InitStruct.Pin = i;
		GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}
static void Joystick_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Pin = JOY_PIN;
	GPIO_InitStruct.Mode = MODE_INPUT;
	GPIO_InitStruct.Pull = PUDPR_PULL_NO;

	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static void Enable_GPIO_Clocks(void) {
	RCC_AHB2ENR |= (EN_GPIOB | EN_GPIOE | EN_GPIOG);
}
static void Hardware_Init(void) {
	// system_stm32l4xx.c util
	SystemInit();

	Enable_GPIO_Clocks();
	Enable_VddIO2_Supply();

	SysTick_Init();

	Display_Init();
	Joystick_Init();
}

static void Read_Inputs(void) {
	count_direction = PIN_IsPressed(JOY_PORT, JOY_PIN) ? -1 : 1;
}

static void Update_Counter(void) {
	static uint32_t last_update = 0;
	if ((system_ticks - last_update) < COUNTER_STEP_MS)
		return;
	last_update = system_ticks;

	current_counter += count_direction;
	if (current_counter > COUNTER_MAX)
		current_counter = COUNTER_MIN;
	else if (current_counter < COUNTER_MIN)
		current_counter = COUNTER_MAX;
}

static void Multiplex_Display(void) {
	static uint32_t last_mux_time = 0;
	static uint8_t counter_digit_index = 0;

	if ((system_ticks - last_mux_time) < MUX_STEP_MS)
		return;
	last_mux_time = system_ticks;

	const uint8_t digit_decmial_values[4] = {
			(current_counter / 1000) % 10,
			(current_counter / 100) % 10,
			(current_counter / 10) % 10,
			(current_counter) % 10
	};

	const uint32_t digit_masks[4] = { DIG1, DIG2, DIG3, DIG4 };

	PORT_ClearMask(GPIOB, ALL_DIGITS_MASK);
	PORT_ClearMask(GPIOG, ALL_SEGMENTS_MASK);

	PORT_SetMask(GPIOG,seven_segment_digit_patterns[digit_decmial_values[counter_digit_index]]);
	PORT_SetMask(GPIOB, digit_masks[counter_digit_index]);

	counter_digit_index = (counter_digit_index + 1) % 4;
}

int main(void) {
	Hardware_Init();

	while (1) {
		Read_Inputs();
		Update_Counter();
		Multiplex_Display();
	}
}
