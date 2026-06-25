#ifndef TIM6_H
#define TIM6_H

#include "my_hal.h"

#define TIM_DIER_UIE_Pos 0U
#define TIM_EGR_UG_Pos   0U
#define TIM_CR1_CEN_Pos  0U
#define TIM_SR_UIF_Pos   0U

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t RESERVED0;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t RESERVED1[3];
    volatile uint32_t CNT;
    volatile uint16_t PSC;
    volatile uint16_t RESERVED2;
    volatile uint16_t ARR;
    volatile uint16_t RESERVED3;
} TIM6_TypeDef;

#define TIM6_BASE   (0x40001000)
#define TIM6        ((TIM6_TypeDef *)TIM6_BASE)
#define TIM_SR_UIF_Msk BIT(TIM_SR_UIF_Pos)

#define NVIC_ISER1     (*((volatile uint32_t *)0xE000E104))
#define NVIC_TIM6_DAC  BIT(22)

static inline void TIM6_Init(uint16_t prescaler, uint16_t period) {
    TIM6->PSC = prescaler;
    TIM6->ARR = period;
    SET_BIT(TIM6->EGR, TIM_EGR_UG_Pos);
    CLR_BIT(TIM6->SR, TIM_SR_UIF_Pos);
    SET_BIT(TIM6->CR1, TIM_CR1_CEN_Pos);
}

static inline bool TIM6_CheckAndClearUpdateFlag(void) {
    if ((TIM6->SR & TIM_SR_UIF_Msk) != 0U) {
        CLR_BIT(TIM6->SR, TIM_SR_UIF_Pos);
        return true;
    }
    return false;
}

static inline void TIM6_EnableInterrupts(void) {
    SET_BIT(TIM6->DIER, TIM_DIER_UIE_Pos);
    NVIC_ISER1 |= NVIC_TIM6_DAC;
}

#endif
