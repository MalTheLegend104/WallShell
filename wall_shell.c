/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */
#include "wall_shell.h"

#ifndef CUSTOM_CONSOLE_COLORS
	#ifdef _WIN32
/* I'm trying to include the least amount of windows headers as possible */
		#include <WinNls.h>
		#include <consoleapi2.h>
		#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		#define SET_TERMINAL_LOCALE
	#endif
/* Modern windows supports these escape codes, older windows versions use SetConsoleTextAttribute */
/* https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences */
	#define RESET_CONSOLE                   printf("\e[27m");
	
	#define SET_CONSOLE_FG_BLACK            printf("\e[30m");
	#define SET_CONSOLE_FG_BRIGHT_BLACK     printf("\e[90m");
	#define SET_CONSOLE_FG_WHITE            printf("\e[37m");
	#define SET_CONSOLE_FG_BRIGHT_WHITE     printf("\e[97m");
	#define SET_CONSOLE_FG_RED              printf("\e[31m");
	#define SET_CONSOLE_FG_BRIGHT_RED       printf("\e[91m");
	#define SET_CONSOLE_FG_GREEN            printf("\e[32m");
	#define SET_CONSOLE_FG_BRIGHT_GREEN     printf("\e[92m");
	#define SET_CONSOLE_FG_YELLOW           printf("\e[32m");
	#define SET_CONSOLE_FG_BRIGHT_YELLOW    printf("\e[93m");
	#define SET_CONSOLE_FG_CYAN             printf("\e[36m");
	#define SET_CONSOLE_FG_BRIGHT_CYAN      printf("\e[96m");
	#define SET_CONSOLE_FG_BLUE             printf("\e[34m");
	#define SET_CONSOLE_FG_BRIGHT_BLUE      printf("\e[94m");
	#define SET_CONSOLE_FG_MAGENTA          printf("\e[35m");
	#define SET_CONSOLE_FG_BRIGHT_MAGENTA   printf("\e[95m");
	
	#define SET_CONSOLE_BG_BLACK            printf("\e[40m");
	#define SET_CONSOLE_BG_BRIGHT_BLACK     printf("\e[100m");
	#define SET_CONSOLE_BG_WHITE            printf("\e[47m");
	#define SET_CONSOLE_BG_BRIGHT_WHITE     printf("\e[107m");
	#define SET_CONSOLE_BG_RED              printf("\e[41m");
	#define SET_CONSOLE_BG_BRIGHT_RED       printf("\e[101m");
	#define SET_CONSOLE_BG_GREEN            printf("\e[42m");
	#define SET_CONSOLE_BG_BRIGHT_GREEN     printf("\e[102m");
	#define SET_CONSOLE_BG_YELLOW           printf("\e[42m");
	#define SET_CONSOLE_BG_BRIGHT_YELLOW    printf("\e[103m");
	#define SET_CONSOLE_BG_CYAN             printf("\e[46m");
	#define SET_CONSOLE_BG_BRIGHT_CYAN      printf("\e[106m");
	#define SET_CONSOLE_BG_BLUE             printf("\e[44m");
	#define SET_CONSOLE_BG_BRIGHT_BLUE      printf("\e[104m");
	#define SET_CONSOLE_BG_MAGENTA          printf("\e[45m");
	#define SET_CONSOLE_BG_BRIGHT_MAGENTA   printf("\e[105m");
#endif // CUSTOM_CONSOLE_COLORS

#ifdef DISABLE_MALLOC

#else
command_t* commands;
#endif

size_t command_count = 0;
char previousCommands[PREVIOUS_BUF_SIZE][MAX_COMMAND_BUF];

uint8_t console_FG_default = CONSOLE_FG_BRIGHT_WHITE;
uint8_t console_BG_default = CONSOLE_BG_BLACK;

uint8_t console_FG_current = CONSOLE_FG_BRIGHT_WHITE;
uint8_t console_BG_current = CONSOLE_BG_BLACK;

void setConsoleLocale() { SET_TERMINAL_LOCALE; }

void setConsoleForegroundDefault(ConsoleColor c) { console_FG_default = c; }
void setConsoleBackgroundDefault(ConsoleColor c) { console_BG_default = c; }

/**
 * @brief Operator overloading isn't available in C. Compare two commands using this.
 *
 * @param c1 Command to compare.
 * @param c2 Command to compare.
 * @return true If they are the same.
 * @return false If they are different.
 */
bool compareCommands(const command_t c1, const command_t c2) {
	if (c1.aliases != c2.aliases) return false;
	if (c1.aliases_count != c2.aliases_count) return false;
	if (strcmp(c1.commandName, c2.commandName) != 0) return false;
	if (c1.helpCommand != c2.helpCommand) return false;
	if (c1.mainCommand != c2.mainCommand) return false;
	return true;
}

void setConsoleColor(ConsoleColor c) {

}

/**
 * @brief Register the command to the command handler.
 * @param c Command to be registered.
 */
void registerCommand(const command_t c) {

}

void deregisterCommand(const command_t c) {

}

void executeCommand(char* commandBuf) {

}


void printGeneralHelp(help_entry_general_t* entry) {

}

void printSpecificHelp(help_entry_specific_t* entry) {

}

void terminalMain() {

}