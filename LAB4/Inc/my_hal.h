#ifndef MY_HAL_H
#define MY_HAL_H

#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1U << (x))
#define SET_BIT(P, B) ((P) |= BIT(B))
#define CLR_BIT(P, B) ((P) &= ~BIT(B))

// Systick
#define STK_BASE             (0xE000E010)
#define STK_CTRL             (*(volatile uint32_t *)(STK_BASE + 0x00))
#define STK_LOAD             (*(volatile uint32_t *)(STK_BASE + 0x04))
#define STK_VAL              (*(volatile uint32_t *)(STK_BASE + 0x08))
#define STK_CTRL_ENABLE      BIT(0)
#define STK_CTRL_TICKINT     BIT(1)
#define STK_CTRL_CLKSOURCE   BIT(2)

typedef struct {
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFRL;
	volatile uint32_t AFRH;
	volatile uint32_t BRR;
	volatile uint32_t ASCR;
} GPIO_TypeDef;

#define GPIOB ((GPIO_TypeDef *)0x48000400)
#define GPIOE ((GPIO_TypeDef *)0x48001000)
#define GPIOG ((GPIO_TypeDef *)0x48001800)

#define RCC_BASE (0x40021000)
#define RCC_APB1ENR1 (*(volatile uint32_t *)(RCC_BASE + 0x58))
#define RCC_AHB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x4C))

#define EN_GPIOB BIT(1)
#define EN_GPIOE BIT(4)
#define EN_GPIOG BIT(6)

#define PWR_BASE (0x40007000)
#define PWR_CR2      (*(volatile uint32_t *)(PWR_BASE + 0x04))

static inline void Enable_VddIO2_Supply(void) {
	SET_BIT(RCC_APB1ENR1, 28);
	SET_BIT(PWR_CR2, 9);
}

#define MODE_INPUT     0
#define MODE_OUTPUT    1
#define MODE_ALTERNATE 2
#define MODE_ANALOG    3

#define OTYPER_TYPE_PP        0
#define OTYPER_TYPE_OD        1


#define OSPEEDR_OUTPUT_LOWSPEED      0
#define OSPEEDR_OUTPUT_MIDSPEED      1
#define OSPEEDR_OUTPUT_HIGHSPEED     2
#define OSPEEDR_OUTPUT_VERYHIGHSPEED 3


#define PUDPR_PULL_NO        0
#define PUDPR_PULL_UP        1
#define PUDPR_PULL_DOWN      2
#define PUDPR_PULL_RESERVED  3

typedef struct {
	uint8_t Pin;
	uint8_t Mode;
	uint8_t Pull;
	uint8_t Speed;
	uint8_t Type;
} GPIO_InitTypeDef;

static inline void GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *init) {
	uint8_t pin = init->Pin;
	uint8_t double_bit_offset = pin * 2;

	port->MODER &= ~(3U << double_bit_offset);
	port->MODER |= (init->Mode << double_bit_offset);


	port->PUPDR &= ~(3U << double_bit_offset);
	port->PUPDR |= (init->Pull << double_bit_offset);


	port->OSPEEDR &= ~(3U << double_bit_offset);
	port->OSPEEDR |= (init->Speed << double_bit_offset);

	if (init->Mode == MODE_OUTPUT) {
		(init->Type == OTYPER_TYPE_OD) ?
				SET_BIT(port->OTYPER, pin) : CLR_BIT(port->OTYPER, pin);
	}
}

static inline void PIN_TurnOn(GPIO_TypeDef *port, uint8_t pin) {
	port->BSRR = BIT(pin);
}

static inline void PIN_TurnOff(GPIO_TypeDef *port, uint8_t pin) {
	port->BSRR = BIT(pin + 16);
}

static inline bool PIN_IsPressed(GPIO_TypeDef *port, uint8_t pin) {
	uint32_t all_pins_current_state = port->IDR;
	uint32_t specific_pin_state = all_pins_current_state & BIT(pin);
	return (specific_pin_state == 0);
}

static inline void PORT_SetMask(GPIO_TypeDef *port, uint32_t mask) {
	port->BSRR = mask;
}

static inline void PORT_ClearMask(GPIO_TypeDef *port, uint32_t mask) {
	port->BSRR = (mask << 16);
}

#endif // MY_HAL_H
