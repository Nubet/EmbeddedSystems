#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include "my_hal.h"

void delay_ms(uint32_t ms_to_wait);

typedef enum {
	LED_ID_D1 = 0,
	LED_ID_D2,
	LED_ID_D3,
	LED_ID_D4,
	LED_ID_D5,
	LED_ID_D6,
	LED_ID_D7,
	LED_ID_D8,
	LED_ID_R,
	LED_ID_G,
	LED_ID_B,
	LED_ID_INVALID
} LedId;

typedef struct {
	GPIO_TypeDef *port;
	uint8_t pin;
} LedPin;

// @formatter:off
static const LedPin LED_GpioMap[11] = {
	{ GPIOC, 6U },
	{ GPIOC, 7U },
	{ GPIOC, 8U },
	{ GPIOC, 9U },
	{ GPIOE, 4U },
	{ GPIOD, 3U },
	{ GPIOE, 5U },
	{ GPIOE, 6U },
	{ GPIOD, 13U },
	{ GPIOB, 8U },
	{ GPIOD, 12U } 
};
// @formatter:on
static inline bool LED_IsValid(LedId led_id) {
	return (led_id >= LED_ID_D1) && (led_id <= LED_ID_B);
}

static inline void LED_Init(void) {
	GPIO_InitTypeDef init = { 0 };
	init.Mode = MODE_OUTPUT;
	init.Pull = PUDPR_PULL_NO;
	init.Speed = OSPEEDR_OUTPUT_LOWSPEED;
	init.Type = OTYPER_TYPE_PP;

	Activate_Clock_For(&RCC_AHB2ENR,
			EN_GPIOB | EN_GPIOC | EN_GPIOD | EN_GPIOE);
	Enable_VddIO2_Supply();

	for (uint8_t led_index = 0U; led_index < 11U; led_index++) {
		init.Pin = LED_GpioMap[led_index].pin;
		GPIO_Init(LED_GpioMap[led_index].port, &init);
		PIN_TurnOff(LED_GpioMap[led_index].port, LED_GpioMap[led_index].pin);
	}
}

static inline void LED_Set(LedId led_id) {
    if (!LED_IsValid(led_id)) {
        return;
    }

    GPIO_TypeDef *port = LED_GpioMap[(uint8_t)led_id].port;
    uint8_t pin        = LED_GpioMap[(uint8_t)led_id].pin;

    PIN_TurnOn(port, pin);
}

static inline void LED_Clear(LedId led_id) {
    if (!LED_IsValid(led_id)) {
        return;
    }

    GPIO_TypeDef *port = LED_GpioMap[(uint8_t)led_id].port;
    uint8_t pin        = LED_GpioMap[(uint8_t)led_id].pin;

    PIN_TurnOff(port, pin);
}

static inline bool LED_Status(LedId led_id) {
    if (!LED_IsValid(led_id)) {
        return false;
    }

    GPIO_TypeDef *port = LED_GpioMap[(uint8_t)led_id].port;
    uint8_t pin        = LED_GpioMap[(uint8_t)led_id].pin;

    return Read_Specific_16Register_Bit(&(port->ODR), pin);
}

static inline void LED_Toggle(LedId led_id) {
	LED_Status(led_id) ? LED_Clear(led_id) : LED_Set(led_id);
}

static inline void LED_Blink(LedId led_id) {
	for (uint8_t blink_count = 0U; blink_count < 5U; blink_count++) {
		LED_Set(led_id);
		delay_ms(200U);
		LED_Clear(led_id);
		delay_ms(200U);
	}
}

#endif // LED_DRIVER_H
