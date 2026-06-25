#ifndef MY_HAL_H
#define MY_HAL_H

#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1U << (x))
#define SET_BIT(P, B) ((P) |= BIT(B))
#define CLR_BIT(P, B) ((P) &= ~BIT(B))

static inline uint8_t Read_Specific_32Register_Bit(volatile uint32_t *reg, uint8_t bit_number) {
    return (*reg & BIT(bit_number)) ? 1U : 0U;
}

static inline uint8_t Read_Specific_16Register_Bit(volatile uint16_t *reg, uint8_t bit_number) {
    return (*reg & BIT(bit_number)) ? 1U : 0U;
}

typedef struct {
    volatile uint32_t MODER;
    volatile uint16_t OTYPER;
    volatile uint16_t RESERVED0;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint16_t IDR;
    volatile uint16_t RESERVED1;
    volatile uint16_t ODR;
    volatile uint16_t RESERVED2;
    volatile uint32_t BSRR;
    volatile uint16_t LCKR;
    volatile uint16_t RESERVED3;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
    volatile uint16_t BRR;
    volatile uint16_t RESERVED4;
    volatile uint16_t ASCR;
    volatile uint16_t RESERVED5;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0x48000000)
#define GPIOB ((GPIO_TypeDef *)0x48000400)
#define GPIOC ((GPIO_TypeDef *)0x48000800)
#define GPIOD ((GPIO_TypeDef *)0x48000C00)
#define GPIOE ((GPIO_TypeDef *)0x48001000)
#define GPIOG ((GPIO_TypeDef *)0x48001800)


#define RCC_BASE (0x40021000)
#define RCC_APB1ENR1 (*(volatile uint32_t *)(RCC_BASE + 0x58))
#define RCC_AHB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x4C))
#define RCC_CCIPR    (*(volatile uint32_t *)(RCC_BASE + 0x88))

#define EN_GPIOA BIT(0)
#define EN_GPIOB BIT(1)
#define EN_GPIOC BIT(2)
#define EN_GPIOD BIT(3)
#define EN_GPIOE BIT(4)
#define EN_GPIOG BIT(6)
#define EN_TIM6  BIT(4)

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

#define GPIO_AF_RESET_Msk (0xFUL)

typedef struct {
    uint8_t Pin;
    uint8_t Mode;
    uint8_t Pull;
    uint8_t Speed;
    uint8_t Type;
    uint8_t Alternate;
} GPIO_InitTypeDef;

static inline void GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *init) {
    uint8_t pin = init->Pin;
    uint8_t double_bit_offset = pin * 2;

    if (init->Mode == MODE_ALTERNATE) {
        if (pin < 8) {
            uint8_t af_offset = pin * 4;
            port->AFRL &= ~(GPIO_AF_RESET_Msk << af_offset);
            port->AFRL |= (init->Alternate << af_offset);
        } else {
            uint8_t af_offset = (pin - 8) * 4;
            port->AFRH &= ~(GPIO_AF_RESET_Msk << af_offset);
            port->AFRH |= (init->Alternate << af_offset);
        }
    }

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

static inline void Activate_Clock_For(volatile uint32_t *reg, uint32_t en_bit_mask) {
    *reg |= en_bit_mask;
}

static inline void PIN_TurnOn(GPIO_TypeDef *port, uint8_t pin) {
    port->BSRR = BIT(pin);
}

static inline void PIN_TurnOff(GPIO_TypeDef *port, uint8_t pin) {
    port->BSRR = BIT(pin + 16);
}

static inline bool PIN_IsPressed(GPIO_TypeDef *port, uint8_t pin) {
	return Read_Specific_16Register_Bit(&(port->IDR), pin) == 0;
}

static inline void PORT_SetMask(GPIO_TypeDef *port, uint32_t mask) {
    port->BSRR = mask;
}

static inline void PORT_ClearMask(GPIO_TypeDef *port, uint32_t mask) {
    port->BSRR = (mask << 16);
}


#endif // MY_HAL_H
