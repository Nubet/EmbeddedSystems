#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "menu.h"
#include "cli.h"
#include "lpuart.h"
#include "my_hal.h"
#include "seven_segment_display.h"
#include "led_driver.h"
#include "joystick.h"

extern uint32_t SystemCoreClock;

typedef enum {
	MENU_TOP = 0,
	MENU_LED,
	MENU_7LED,
	MENU_JOY,
	MENU_LPUART
} MenuLevel;

static MenuLevel current_menu_context = MENU_TOP;

static char ConvertCharToUpperCase(char c) {
	if (c >= 'a' && c <= 'z') {
		return (char) (c - ('a' - 'A'));
	}
	return c;
}

static const char* SkipStringLeadingSpaces(const char *text) {
	while (*text == ' ') {
		text++;
	}
	return text;
}

static bool IsEndOfString(const char *text) {
	return *SkipStringLeadingSpaces(text) == '\0';
}

static bool AreBothStringsEqualRegardlessOfCase(const char *a, const char *b) {
	while (*a != '\0' && *b != '\0') {
		if (ConvertCharToUpperCase(*a) != ConvertCharToUpperCase(*b)) {
			return false;
		}
		a++;
		b++;
	}
	return (*a == '\0' && *b == '\0');
}

static bool String_DoesLineStartWithThisCommandRegardlessOfCase(const char *text,
		const char *prefix) {
	// Loops until the end of prefix (command). So if input is "Read R", it matches "READ" and ignores the space and "R".
	while (*prefix != '\0') {
		if (*text == '\0') {
			return false;
		}
		if (ConvertCharToUpperCase(*text) != ConvertCharToUpperCase(*prefix)) {
			return false;
		}
		text++;
		prefix++;
	}
	return true;
}

static int ExtractValidNumericValueFromText(const char *text) {
	int value = 0;

	if (*text == '\0') {
		return -1;
	}

	while (*text != '\0') {
		if (*text < '0' || *text > '9') {
			return -1;
		}
		if (value > 99999) {
			return -1;
		}
		value = (value * 10) + (*text - '0');
		text++;
	}

	return value;
}

static LedId ConvertTextToHardwareLedIdentifier(const char *token) {
	if (token == NULL || token[0] == '\0' || token[1] != '\0') {
        return LED_ID_INVALID;
    }
	if (token[0] >= '0' && token[0] <= '7') {
		return (LedId) (token[0] - '0');
	}
	if (ConvertCharToUpperCase(token[0]) == 'R') {
		return LED_ID_R;
	}
	if (ConvertCharToUpperCase(token[0]) == 'G') {
		return LED_ID_G;
	}
	if (ConvertCharToUpperCase(token[0]) == 'B') {
		return LED_ID_B;
	}
	
	return LED_ID_INVALID;
}

static JoyId String_ConvertTextToHardwareJoystickButtonIdentifier(
		const char *token) {

	if (token == NULL || token[0] == '\0' || token[1] != '\0') {
		return JOY_INVALID;
	}
	if (ConvertCharToUpperCase(token[0]) == 'L') {
		return JOY_LEFT;
	}
	if (ConvertCharToUpperCase(token[0]) == 'R') {
		return JOY_RIGHT;
	}
	if (ConvertCharToUpperCase(token[0]) == 'U') {
		return JOY_UP;
	}
	if (ConvertCharToUpperCase(token[0]) == 'D') {
		return JOY_DOWN;
	}
	if (ConvertCharToUpperCase(token[0]) == 'C') {
		return JOY_CENTER;
	}
	return JOY_INVALID;
}

static void SendHelpTop(void) {
	CLI_Print("Available commands:\r\n");
	CLI_Print("  LED     - enter LED menu\r\n");
	CLI_Print("  7LED    - enter 7-segment display menu\r\n");
	CLI_Print("  JOY     - enter joystick menu\r\n");
	CLI_Print("  LPUART  - enter LPUART menu\r\n");
	CLI_Print("  HELP    - show this help\r\n");
}

static void SendHelpLed(void) {
	CLI_Print("LED commands:\r\n");
	CLI_Print("  SET <id>     - turn LED on\r\n");
	CLI_Print("  CLEAR <id>   - turn LED off\r\n");
	CLI_Print("  TOGGLE <id>  - invert LED state\r\n");
	CLI_Print("  BLINK <id>   - blink LED 5 times\r\n");
	CLI_Print("  STATUS <id>  - show LED state\r\n");
	CLI_Print("  UP         - return to top menu\r\n");
	CLI_Print("  HELP         - show this help\r\n");
	CLI_Print("Valid <id>: 0-7, R, G, B\r\n");
}

static void SendHelp7Led(void) {
	CLI_Print("7LED commands:\r\n");
	CLI_Print("  DISPLAY <value>  - show number 0-9999\r\n");
	CLI_Print("  READ             - read current value\r\n");
	CLI_Print("  UP             - return to top menu\r\n");
	CLI_Print("  HELP             - show this help\r\n");
}

static void SendHelpJoy(void) {
	CLI_Print("JOY commands:\r\n");
	CLI_Print("  READ <id>  - read joystick button\r\n");
	CLI_Print("  UP       - return to top menu\r\n");
	CLI_Print("  HELP       - show this help\r\n");
	CLI_Print("Valid <id>: L, R, U, D, C\r\n");
}

static void SendHelpLpuart(void) {
	CLI_Print("LPUART commands:\r\n");
	CLI_Print("  STATUS  - show port configuration from registers\r\n");
	CLI_Print("  UP    - return to top menu\r\n");
	CLI_Print("  HELP    - show this help\r\n");
}

static bool Menu_ProcessGlobalCommandsHelpAndExit(const char *line,
		void (*help_function)(void)) {
	if (AreBothStringsEqualRegardlessOfCase(line, "HELP")) {
		help_function();
		return true;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "UP")) {
		current_menu_context = MENU_TOP;
		return true;
	}
	return false;
}

static bool Menu_ValidateCommandAndExtractTargetLed(const char *line,
		const char *command,
		uint8_t command_length, LedId *out_led_id) {
	const char *arg = 0;

	if (!String_DoesLineStartWithThisCommandRegardlessOfCase(line, command)) {
		return false;
	}

	arg = SkipStringLeadingSpaces(line + command_length);
	*out_led_id = ConvertTextToHardwareLedIdentifier(arg);
	if (!LED_IsValid(*out_led_id)
			|| !IsEndOfString(arg + 1)) {
		CLI_Print("Invalid LED id\r\n");
		return true;
	}

	return true;
}

static void ExecuteLedCommand(char *line) {
	LedId led_id = LED_ID_INVALID;

	if (Menu_ProcessGlobalCommandsHelpAndExit(line, SendHelpLed)) {
		return;
	}

	if (Menu_ValidateCommandAndExtractTargetLed(line, "SET", 3U, &led_id)) {
		if (!LED_IsValid(led_id)) {
			return;
		}
		LED_Set(led_id);

		CLI_Print("OK\r\n");
		return;
	}
	if (Menu_ValidateCommandAndExtractTargetLed(line, "CLEAR", 5U, &led_id)) {
		if (!LED_IsValid(led_id)) {
			return;
		}
		LED_Clear(led_id);
		CLI_Print("OK\r\n");
		return;
	}
	if (Menu_ValidateCommandAndExtractTargetLed(line, "TOGGLE", 6U, &led_id)) {
		if (!LED_IsValid(led_id)) {
			return;
		}
		LED_Toggle(led_id);
		CLI_Print("OK\r\n");
		return;
	}
	if (Menu_ValidateCommandAndExtractTargetLed(line, "BLINK", 5U, &led_id)) {
		if (!LED_IsValid(led_id)) {
			return;
		}
		LED_Blink(led_id);
		CLI_Print("OK\r\n");
		return;
	}
	if (Menu_ValidateCommandAndExtractTargetLed(line, "STATUS", 6U, &led_id)) {
		if (!LED_IsValid(led_id)) {
			return;
		}
		CLI_Print(LED_Status(led_id) ? "ON\r\n" : "OFF\r\n");
		return;
	}
	CLI_Print("Unknown command\r\n");
}

static void Execute7LedCommand(char *line) {
	if (Menu_ProcessGlobalCommandsHelpAndExit(line, SendHelp7Led)) {
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "READ")) {
		uint16_t value = Display_GetNumber();
		CLI_PrintUint32(value);
		CLI_Print("\r\n");
		return;
	}
	if (String_DoesLineStartWithThisCommandRegardlessOfCase(line, "DISPLAY")) {
		const char *arg = SkipStringLeadingSpaces(line + 7);
		int parsed = ExtractValidNumericValueFromText(arg);
		if (parsed < 0 || parsed > 9999) {
			CLI_Print("Invalid value\r\n");
			return;
		}
		Display_SetNumber((uint16_t) parsed);
		CLI_Print("OK\r\n");
		return;
	}
	CLI_Print("Unknown command\r\n");
}

static void ExecuteJoyCommand(char *line) {
	if (Menu_ProcessGlobalCommandsHelpAndExit(line, SendHelpJoy)) {
		return;
	}
	if (String_DoesLineStartWithThisCommandRegardlessOfCase(line, "READ")) {
		const char *arg = SkipStringLeadingSpaces(line + 4);
		JoyId joy_id =
				String_ConvertTextToHardwareJoystickButtonIdentifier(arg);
		if (!JOY_IsValid(joy_id)
				|| !IsEndOfString(arg + 1)) {
			CLI_Print("Invalid joystick id\r\n");
			return;
		}
		CLI_Print(JOY_IsPressed(joy_id) ? "PRESSED\r\n" : "RELEASED\r\n");
		return;
	}
	CLI_Print("Unknown command\r\n");
}

static void ExecuteLpuartCommand(char *line) {
	if (Menu_ProcessGlobalCommandsHelpAndExit(line, SendHelpLpuart)) {
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "STATUS")) {
		uint32_t cr1 = MY_LPUART1->CR1;
		uint32_t cr2 = MY_LPUART1->CR2;
		uint32_t brr = MY_LPUART1->BRR;
		uint32_t calculated_baud = (brr == 0U) ? 0U :
				(256UL * SystemCoreClock) / brr;

		// word length control bits
		uint8_t m0 = Read_Specific_32Register_Bit(&cr1, LPUART_CR1_M0_Pos);
		uint8_t m1 = Read_Specific_32Register_Bit(&cr1, LPUART_CR1_M1_Pos);

		// PCE (Parity Control Enable): 1 = Enabled, 0 = Disabled
		uint8_t pce = Read_Specific_32Register_Bit(&cr1, LPUART_CR1_PCE_Pos);
		// PS (Parity Selection): 0 = Even parity, 1 = Odd parity
		uint8_t ps = Read_Specific_32Register_Bit(&cr1, LPUART_CR1_PS_Pos);

		uint32_t stop_bits = (cr2 & LPUART_CR2_STOP_Msk) >> LPUART_CR2_STOP_Pos;
		uint32_t databits = 8U;

		if (m1 == 0U && m0 == 1U) {
			databits = 9U;
		} else if (m1 == 1U && m0 == 0U) {
			databits = 7U;
		}

		CLI_Print("Baudrate(calc): ");
		CLI_PrintUint32(calculated_baud);
		CLI_Print("\r\n");
		CLI_Print("Databits: ");
		CLI_PrintUint32(databits);
		CLI_Print("\r\n");
		CLI_Print("Parity: ");
		if (pce == 0U) {
			CLI_Print("none");
		} else if (ps == 0U) {
			CLI_Print("even");
		} else {
			CLI_Print("odd");
		}
		CLI_Print("\r\n");
		CLI_Print("Stop bits: ");
		if (stop_bits == 0U) {
			CLI_Print("1");
		} else if (stop_bits == 1U) {
			CLI_Print("0.5");
		} else if (stop_bits == 2U) {
			CLI_Print("2");
		} else {
			CLI_Print("1.5");
		}
		CLI_Print("\r\n");
		CLI_Print("BRR register: ");
		CLI_PrintUint32(brr);
		CLI_Print("\r\n");
		return;
	}
	CLI_Print("Unknown command\r\n");
}

static void RouteTopLevelNavigation(char *line) {
	if (AreBothStringsEqualRegardlessOfCase(line, "HELP")) {
		SendHelpTop();
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "LED")) {
		current_menu_context = MENU_LED;
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "7LED")) {
		current_menu_context = MENU_7LED;
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "JOY")) {
		current_menu_context = MENU_JOY;
		return;
	}
	if (AreBothStringsEqualRegardlessOfCase(line, "LPUART")) {
		current_menu_context = MENU_LPUART;
		return;
	}
	CLI_Print("Unknown command\r\n");
}

void Menu_Init(void) {
	current_menu_context = MENU_TOP;
}

void Menu_PrintPromptForCurrentContext(void) {
	if (current_menu_context == MENU_TOP) {
		CLI_Print("> ");
		return;
	}
	if (current_menu_context == MENU_LED) {
		CLI_Print("LED> ");
		return;
	}
	if (current_menu_context == MENU_7LED) {
		CLI_Print("7LED> ");
		return;
	}
	if (current_menu_context == MENU_JOY) {
		CLI_Print("JOY> ");
		return;
	}
	CLI_Print("LPUART> ");
}

void Menu_ExecuteCommandInCurrentContext(char *line) {
	char *trimmed = (char*) SkipStringLeadingSpaces(line);

	if (*trimmed == '\0') {
		return;
	}

	if (current_menu_context == MENU_TOP) {
		RouteTopLevelNavigation(trimmed);
		return;
	}
	if (current_menu_context == MENU_LED) {
		ExecuteLedCommand(trimmed);
		return;
	}
	if (current_menu_context == MENU_7LED) {
		Execute7LedCommand(trimmed);
		return;
	}
	if (current_menu_context == MENU_JOY) {
		ExecuteJoyCommand(trimmed);
		return;
	}
	ExecuteLpuartCommand(trimmed);
}
