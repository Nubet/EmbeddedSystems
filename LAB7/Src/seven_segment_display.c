#include "seven_segment_display.h"

#define SEG_A BIT(0)
#define SEG_B BIT(1)
#define SEG_C BIT(2)
#define SEG_D BIT(3)
#define SEG_E BIT(4)
#define SEG_F BIT(5)
#define SEG_G BIT(6)
#define SEG_DP BIT(9)

#define ALL_SEGMENTS_MASK (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP)

#define DIG1 BIT(2)
#define DIG2 BIT(3)
#define DIG3 BIT(4)
#define DIG4 BIT(5)

#define ALL_DIGITS_MASK (DIG1 | DIG2 | DIG3 | DIG4)

static const uint32_t digit_masks[4] = {
DIG1, DIG2, DIG3, DIG4 };

// @formatter:off
static const uint16_t seven_segment_digit_patterns[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          // 0
    SEG_B | SEG_C,                                          // 1
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                  // 2
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                  // 3
    SEG_B | SEG_C | SEG_F | SEG_G,                          // 4
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                  // 5
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,          // 6
    SEG_A | SEG_B | SEG_C,                                  // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,  // 8
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G           // 9
};
// @formatter:on

static uint16_t Display_GetSegmentsForCharacter(char character) {
	if (character >= '0' && character <= '9') {
		return seven_segment_digit_patterns[(uint8_t) (character - '0')];
	}
	return 0U;
}

void Display_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	Activate_Clock_For(&RCC_AHB2ENR, EN_GPIOB | EN_GPIOG);
	Enable_VddIO2_Supply();

	GPIO_InitStruct.Mode = MODE_OUTPUT;
	GPIO_InitStruct.Pull = PUDPR_PULL_NO;
	GPIO_InitStruct.Speed = OSPEEDR_OUTPUT_LOWSPEED;
	GPIO_InitStruct.Type = OTYPER_TYPE_PP;

	for (uint8_t i = 0; i <= 6; i++) {
		GPIO_InitStruct.Pin = i;
		GPIO_Init(GPIOG, &GPIO_InitStruct);
	}

	GPIO_InitStruct.Pin = 9;
	GPIO_Init(GPIOG, &GPIO_InitStruct);

	for (uint8_t i = 2; i <= 5; i++) {
		GPIO_InitStruct.Pin = i;
		GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

	PORT_ClearMask(GPIOB, ALL_DIGITS_MASK);
	PORT_ClearMask(GPIOG, ALL_SEGMENTS_MASK);
}

void Display_RenderCurrentState(char visible_character) {

	uint16_t segment_mask = Display_GetSegmentsForCharacter(visible_character);

	PORT_ClearMask(GPIOB, ALL_DIGITS_MASK);
	PORT_ClearMask(GPIOG, ALL_SEGMENTS_MASK);

	if (segment_mask != 0U) {
		PORT_SetMask(GPIOG, (uint32_t) segment_mask);
	}

	PORT_SetMask(GPIOB, digit_masks[ACTIVE_DIGIT_INDEX]);
}
