#include "cli.h"
#include "lpuart.h"

#define ASCII_BACKSPACE     8U
#define ASCII_DELETE        127U


static char incoming_line_buffer[CLI_LINE_BUFFER_SIZE];
static uint8_t current_line_length = 0U;
static bool line_full_warned = false;


static inline bool IsLineTerminator(uint8_t character) {
    return (character == '\r' || character == '\n');
}

static inline bool IsBackspaceOrDelete(uint8_t character) {
    return (character == ASCII_BACKSPACE || character == ASCII_DELETE);
}


static void CopyBufferToCommandStringAndReset(char *assembled_command_string) {
    CLI_Print(ANSI_CURSOR_SHOW);
    CLI_Print("\r\n");
    line_full_warned = false;

    for (uint8_t i = 0U; i < current_line_length; i++) {
        assembled_command_string[i] = incoming_line_buffer[i];
    }

    assembled_command_string[current_line_length] = '\0';
    current_line_length = 0U;
}

static void EraseLastCharacterFromBufferAndScreen(void) {
    if (current_line_length > 0U) {
        current_line_length--;
        line_full_warned = false;
        CLI_Print(ANSI_CURSOR_SHOW);
        CLI_Print("\b \b");
    }
}

static void AppendCharacterToBufferOrWarnIfFull(uint8_t received_character) {
    if (current_line_length < (CLI_LINE_BUFFER_SIZE - 1U)) {
        incoming_line_buffer[current_line_length++] = (char) received_character;
        LPUART_SendChar(received_character);
    } else if (!line_full_warned) {
        line_full_warned = true;
        LPUART_SendChar((unsigned char)'\a');
        CLI_Print(ANSI_CURSOR_HIDE);
    } else {
        LPUART_SendChar((unsigned char)'\a');
    }
}


uint8_t ConvertUint32ToString(uint32_t value, char *out_buffer) {
    uint8_t length = 0U;

    do {
        out_buffer[length++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value > 0U);

    out_buffer[length] = '\0'; 

    for (uint8_t i = 0U; i < length / 2U; i++) {
        uint8_t left_index  = i;
        uint8_t right_index = length - 1U - i;

        char temp_char           = out_buffer[left_index];
        out_buffer[left_index]   = out_buffer[right_index];
        out_buffer[right_index]  = temp_char;
    }

    return length;
}

void CLI_Print(const char *text) {
    LPUART_SendString((unsigned char*) text);
}

void CLI_PrintUint32(uint32_t value) {
    char str_buffer[MAX_UINT32_DIGITS + 1U];

    ConvertUint32ToString(value, str_buffer);
    LPUART_SendString((unsigned char*) str_buffer);
}

bool CLI_CheckForIncomingCharacterAndAssembleLine(char *assembled_command_string) {
    uint8_t received_character = 0U;

    if (LPUART_TryReadByte(&received_character) != 0) {
        return false;
    }

    if (IsLineTerminator(received_character)) {
        CopyBufferToCommandStringAndReset(assembled_command_string);
        return true;
    }

    if (IsBackspaceOrDelete(received_character)) {
        EraseLastCharacterFromBufferAndScreen();
        return false;
    }

    AppendCharacterToBufferOrWarnIfFull(received_character);

    return false;
}
