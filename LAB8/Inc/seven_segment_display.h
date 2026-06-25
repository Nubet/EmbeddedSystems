#ifndef SEVEN_SEGMENT_DISPLAY_H
#define SEVEN_SEGMENT_DISPLAY_H

#include <stdint.h>
#include "my_hal.h"

void Display_Init(void);
void Display_SetNumber(uint16_t value);
uint16_t Display_GetNumber(void);
void Display_RenderCurrentState(void);

#endif // SEVEN_SEGMENT_DISPLAY_H
