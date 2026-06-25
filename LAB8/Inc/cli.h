#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdint.h>

#define CLI_LINE_BUFFER_SIZE 32U

// - uint32_t max value: 4,294,967,295 (10 digits)
// - int32_t min value: -2,147,483,648 (11 characters including minus sign)
// 11U serves as a safe maximum length for any 32-bit integer string representation
#define MAX_UINT32_DIGITS   11U

#define ANSI_CURSOR_HIDE "\033[?25l"
#define ANSI_CURSOR_SHOW "\033[?25h"


uint8_t ConvertUint32ToString(uint32_t value, char *out_buffer);

void CLI_Print(const char *text);
void CLI_PrintUint32(uint32_t value);

bool CLI_CheckForIncomingCharacterAndAssembleLine(char *assembled_command_string);

#endif // CLI_H
