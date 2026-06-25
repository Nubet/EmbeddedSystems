#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "my_hal.h"

typedef enum {
	JOY_LEFT = 0,
	JOY_RIGHT,
	JOY_UP,
	JOY_DOWN,
	JOY_CENTER,
	JOY_INVALID
} JoyId;

typedef struct {
	GPIO_TypeDef *port;
	uint8_t pin;
} JoyPin;

// @formatter:off
static const JoyPin JOY_GpioMap[5] = { 
	{ GPIOE, 1U },
	{ GPIOE, 0U },
	{ GPIOE, 3U },
	{ GPIOE, 2U },
	{ GPIOE, 15U } 
};
// @formatter:on

static inline bool JOY_IsValid(JoyId joy_id) {
	return (joy_id >= JOY_LEFT) && (joy_id <= JOY_CENTER);
}

static inline void JOY_Init(void) {
	GPIO_InitTypeDef init = { 0 };
	init.Mode = MODE_INPUT;
	init.Pull = PUDPR_PULL_NO;
	init.Speed = OSPEEDR_OUTPUT_LOWSPEED;
	init.Type = OTYPER_TYPE_PP;

	Activate_Clock_For(&RCC_AHB2ENR, EN_GPIOE);

	for (uint8_t joystick_index = 0U; joystick_index < 5U;
			joystick_index++) {
		init.Pin = JOY_GpioMap[joystick_index].pin;
		GPIO_Init(JOY_GpioMap[joystick_index].port, &init);
	}
}

static inline bool JOY_IsPressed(JoyId joy_id) {
	if (!JOY_IsValid(joy_id)) {
		return false;
	}
	return PIN_IsPressed(JOY_GpioMap[(uint8_t) joy_id].port,
			JOY_GpioMap[(uint8_t) joy_id].pin);
}

#endif // JOYSTICK_H
