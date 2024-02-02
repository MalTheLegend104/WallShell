/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/*********************************************************************************
 * Check for user definitions
 *********************************************************************************/
#ifndef PREVIOUS_BUF_SIZE
	#define PREVIOUS_BUF_SIZE 50
#endif // PREVIOUS_BUF_SIZE

#ifndef MAX_COMMAND_BUF
	#define MAX_COMMAND_BUF 256
#endif // MAX_COMMAND_BUF

#ifndef SHELL_COMMAND_PREFIX
	#define SHELL_COMMAND_PREFIX ">"
#endif // SHELL_COMMAND_PREFIX

#ifdef THREADED_SUPPORT
// Add supported for threaded applications
#endif // THREADED_SUPPORT

#ifdef DISABLE_MALLOC
// We're going to develop it for malloc first before implementing this
#define COMMAND_LIMIT 25
#endif // DISABLE_MALLOC

/* Freestanding headers. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Standard Library Headers */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Command type, holds the function pointer, it's help function pointer, command name, aliases, and amount of aliases.
 */
typedef struct {
	int (* mainCommand)(int argc, char** argv);
	int (* helpCommand)(int argc, char** argv);
	const char* commandName;
	const char** aliases;
	size_t aliases_count;
} command_t;

/**
 * @brief General help structure. Makes printing help menu entries consistent across different functions.
 * Not all fields have to be filled in. You can leave fields as NULL to not print them.
 *
 * This is meant for general command help entries, mostly meaning first level commands (like clear, exit, etc.)
 */
typedef struct {
	const char* commandName;
	const char* description;
	const char** commands;
	const int commands_count;
	const char** aliases;
	const int aliases_count;
} help_entry_general_t;

/**
 * @brief Specific help structure. Makes printing help menu entries consistent across different functions.
 * Not all fields have to be filled in. You can leave fields as NULL to not print them.
 *
 * This is meant for things like subcommands, flags, etc.
 */
typedef struct {
	const char* commandName;
	const char* description;
	const char** required;
	const int required_count;
	const char** optional;
	const int optional_count;
} help_entry_specific_t;

typedef enum {
	CONSOLE_COLOR_BLACK,
	CONSOLE_COLOR_BRIGHT_BLACK,
	CONSOLE_COLOR_WHITE,
	CONSOLE_COLOR_BRIGHT_WHITE,
	CONSOLE_COLOR_RED,
	CONSOLE_COLOR_BRIGHT_RED,
	CONSOLE_COLOR_GREEN,
	CONSOLE_COLOR_BRIGHT_GREEN,
	CONSOLE_COLOR_YELLOW,
	CONSOLE_COLOR_BRIGHT_YELLOW,
	CONSOLE_COLOR_CYAN,
	CONSOLE_COLOR_BRIGHT_CYAN,
	CONSOLE_COLOR_BLUE,
	CONSOLE_COLOR_BRIGHT_BLUE,
	CONSOLE_COLOR_MAGENTA,
	CONSOLE_COLOR_BRIGHT_MAGENTA
} ConsoleColor;

bool compareCommands(const command_t c1, const command_t c2);

void registerCommand(const command_t c);
void deregisterCommand(const command_t c);
void executeCommand(char* commandBuf);
void terminalMain();

void printGeneralHelp(help_entry_general_t* entry);
void printSpecificHelp(help_entry_specific_t* entry);

#endif // COMMAND_HANDLER_H
