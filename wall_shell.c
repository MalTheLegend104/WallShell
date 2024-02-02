/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */
#include "wall_shell.h"

#ifndef CUSTOM_CONSOLE_COLORS
	#ifdef _WIN32
		#include <Windows.h>

		#define SET_CONSOLE_COLOR_BLACK             SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED); // Grey
		#define SET_CONSOLE_COLOR_RED               SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		#define SET_CONSOLE_COLOR_BRIGHT_RED        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		#define SET_CONSOLE_COLOR_GREEN             SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		#define SET_CONSOLE_COLOR_BRIGHT_GREEN      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		#define SET_CONSOLE_COLOR_YELlOW            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
		#define SET_CONSOLE_COLOR_BRIGHT_YELlOW     SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		#define SET_CONSOLE_COLOR_CYAN              SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN);
		#define SET_CONSOLE_COLOR_BRIGHT_CYAN       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		#define SET_CONSOLE_COLOR_BLUE              SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE );
		#define SET_CONSOLE_COLOR_BRIGHT_BLUE       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		#define SET_CONSOLE_COLOR_MAGENTA           SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED);
		#define SET_CONSOLE_COLOR_BRIGHT_MAGENTA    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);

		#define SET_TERMINAL_LOCALE                 SetConsoleOutputCP(CP_UTF8);
	
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
/* https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences */
		#define SET_CONSOLE_COLOR_BLACK             printf("\e[0m");
		#define SET_CONSOLE_COLOR_BRIGHT_BLACK      printf("\e[0m");
		#define SET_CONSOLE_COLOR_WHITE             printf("\e[0m");
		#define SET_CONSOLE_COLOR_BRIGHT_WHITE      printf("\e[0m");
		#define SET_CONSOLE_COLOR_RED               printf("\e[31m");
		#define SET_CONSOLE_COLOR_BRIGHT_RED        printf("\e[91m");
		#define SET_CONSOLE_COLOR_GREEN             printf("\e[32m");
		#define SET_CONSOLE_COLOR_BRIGHT_GREEN      printf("\e[92m");
		#define SET_CONSOLE_COLOR_YELLOW            printf("\e[32m");
		#define SET_CONSOLE_COLOR_BRIGHT_YELLOW     printf("\e[93m");
		#define SET_CONSOLE_COLOR_CYAN              printf("\e[36m");
		#define SET_CONSOLE_COLOR_BRIGHT_CYAN       printf("\e[96m");
		#define SET_CONSOLE_COLOR_BLUE              printf("\e[34m");
		#define SET_CONSOLE_COLOR_BRIGHT_BLUE       printf("\e[94m");
		#define SET_CONSOLE_COLOR_MAGENTA           printf("\e[35m");
		#define SET_CONSOLE_COLOR_BRIGHT_MAGENTA    printf("\e[95m");
		
		#define SET_TERMINAL_LOCALE ;
	#endif
#endif // CUSTOM_CONSOLE_COLORS

uint8_t console_FG_default = 0;
uint8_t console_BG_default = 0;

uint8_t console_FG_current = 0;
uint8_t console_BG_current = 0;

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