#include "seven_segment_display.h"

#define DIS_SEGMENTS_PORT   GPIOG
#define DIS_DIGITS_PORT     GPIOB

#define DISPLAY_RCC_EN_MASKS (EN_GPIOB | EN_GPIOG)

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

static const uint32_t digit_masks[4] = { DIG1, DIG2, DIG3, DIG4 };

// Internal module state
static uint16_t stored_value        = 0U;
static char     digits_buffer[4]    = { '0', '0', '0', '0' };
static uint8_t  current_digit_index = 0U;

// @formatter:off
static const uint16_t seven_segment_digit_patterns[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,           // 0
    SEG_B | SEG_C,                                           // 1
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // 2
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                   // 3
    SEG_B | SEG_C | SEG_F | SEG_G,                           // 4
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // 5
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,           // 6
    SEG_A | SEG_B | SEG_C,                                   // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   // 8
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G            // 9
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

    Activate_Clock_For(&RCC_AHB2ENR, DISPLAY_RCC_EN_MASKS);
    Enable_VddIO2_Supply();

    GPIO_InitStruct.Mode = MODE_OUTPUT;
    GPIO_InitStruct.Pull = PUDPR_PULL_NO;
    GPIO_InitStruct.Speed = OSPEEDR_OUTPUT_LOWSPEED;
    GPIO_InitStruct.Type = OTYPER_TYPE_PP;

    for (uint8_t segment_pin = 0U; segment_pin <= 6U; segment_pin++) {
        GPIO_InitStruct.Pin = segment_pin;
        GPIO_Init(DIS_SEGMENTS_PORT, &GPIO_InitStruct);
    }

    GPIO_InitStruct.Pin = 9;
    GPIO_Init(DIS_SEGMENTS_PORT, &GPIO_InitStruct);

    for (uint8_t digit_pin = 2U; digit_pin <= 5U; digit_pin++) {
        GPIO_InitStruct.Pin = digit_pin;
        GPIO_Init(DIS_DIGITS_PORT, &GPIO_InitStruct);
    }

    PORT_ClearMask(DIS_DIGITS_PORT, ALL_DIGITS_MASK);
    PORT_ClearMask(DIS_SEGMENTS_PORT, ALL_SEGMENTS_MASK);
}

static uint16_t Display_ClampValueToFourDigits(uint16_t value) {
    if (value > 9999U) {
        return 9999U;
    }
    return value;
}

static void Display_ClearDigitBuffer(char *buffer, uint8_t size) {
    for (uint8_t i = 0U; i < size; i++) {
        buffer[i] = ' ';
    }
}

void Display_SetNumber(uint16_t value) {
    stored_value = Display_ClampValueToFourDigits(value);
    Display_ClearDigitBuffer(digits_buffer, 4U);

    uint16_t remaining_value = stored_value;
    uint8_t buffer_position = 3U; // rightmost

    do {
        uint8_t single_digit = remaining_value % 10U;

        digits_buffer[buffer_position] = (char)('0' + single_digit);

        if (buffer_position > 0U) buffer_position--;
        remaining_value /= 10U;

    } while (remaining_value > 0U);
}

uint16_t Display_GetNumber(void) {
    return stored_value;
}

static char Display_GetCharacterForDigit(uint8_t digit_index) {
    return digits_buffer[digit_index];
}

void Display_RenderCurrentState(void) {
    char visible_character = Display_GetCharacterForDigit(current_digit_index);
    uint16_t segment_mask = Display_GetSegmentsForCharacter(visible_character);

    PORT_ClearMask(DIS_DIGITS_PORT, ALL_DIGITS_MASK);
    PORT_ClearMask(DIS_SEGMENTS_PORT, ALL_SEGMENTS_MASK);

    // Avoid writing empty masks
    if (segment_mask != 0U) {
        PORT_SetMask(DIS_SEGMENTS_PORT, (uint32_t) segment_mask);
    }

    PORT_SetMask(DIS_DIGITS_PORT, digit_masks[current_digit_index]);

    current_digit_index++;
    if (current_digit_index >= 4U) {
        current_digit_index = 0U;
    }
}
