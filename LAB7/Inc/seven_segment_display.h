#ifndef SEVEN_SEGMENT_DISPLAY_H
#define SEVEN_SEGMENT_DISPLAY_H

#include <stdint.h>
#include "my_hal.h"

#define ACTIVE_DIGIT_INDEX 0U

void Display_Init(void);
void Display_RenderCurrentState(char visible_character);

#endif // SEVEN_SEGMENT_DISPLAY_H
