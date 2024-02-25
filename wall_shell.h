/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/* Freestanding headers. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Standard Library Headers */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Config Header */
#include "wallshell_config.h"

/*********************************************************************************
 * Check for user definitions
 *********************************************************************************/
#ifndef PREVIOUS_BUF_SIZE
	#define PREVIOUS_BUF_SIZE 50
#endif // PREVIOUS_BUF_SIZE

#ifndef MAX_COMMAND_BUF
	#define MAX_COMMAND_BUF 256
#endif // MAX_COMMAND_BUF

#ifdef THREADED_SUPPORT
// Add supported for threaded applications
#endif // THREADED_SUPPORT

#ifdef DISABLE_MALLOC
// We're going to develop it for malloc first before implementing this
	#ifndef COMMAND_LIMIT
		#define COMMAND_LIMIT 25
	#endif
	#ifndef MAX_ARGS
		#define MAX_ARGS 32
	#endif
#endif // DISABLE_MALLOC

typedef enum {
	WALLSHELL_NO_ERROR = 0,
	WALLSHELL_OUT_OF_MEMORY,
	WALLSHELL_COMMAND_LIMIT_REACHED,
	WALLSHELL_OUT_STREAM_NOT_SET,
	WALLSHELL_CANT_SET_DEFAULT_TO_DEFAULT,
	WALLSHELL_CONSOLE_SETUP_ERROR
} wallshell_error_t;

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
	CONSOLE_FG_DEFAULT = 0,
	CONSOLE_FG_BLACK = 30,
	CONSOLE_FG_RED = 31,
	CONSOLE_FG_GREEN = 32,
	CONSOLE_FG_YELLOW = 33,
	CONSOLE_FG_BLUE = 34,
	CONSOLE_FG_MAGENTA = 35,
	CONSOLE_FG_CYAN = 36,
	CONSOLE_FG_WHITE = 37,
	
	CONSOLE_FG_BRIGHT_BLACK = 90,
	CONSOLE_FG_BRIGHT_RED = 91,
	CONSOLE_FG_BRIGHT_GREEN = 92,
	CONSOLE_FG_BRIGHT_YELLOW = 93,
	CONSOLE_FG_BRIGHT_BLUE = 94,
	CONSOLE_FG_BRIGHT_MAGENTA = 95,
	CONSOLE_FG_BRIGHT_CYAN = 96,
	CONSOLE_FG_BRIGHT_WHITE = 97,
} console_fg_color_t;

typedef enum {
	CONSOLE_BG_DEFAULT = 0,
	CONSOLE_BG_BLACK = 40,
	CONSOLE_BG_RED = 41,
	CONSOLE_BG_GREEN = 42,
	CONSOLE_BG_YELLOW = 43,
	CONSOLE_BG_BLUE = 44,
	CONSOLE_BG_MAGENTA = 45,
	CONSOLE_BG_CYAN = 46,
	CONSOLE_BG_WHITE = 47,
	
	CONSOLE_BG_BRIGHT_BLACK = 100,
	CONSOLE_BG_BRIGHT_RED = 101,
	CONSOLE_BG_BRIGHT_GREEN = 102,
	CONSOLE_BG_BRIGHT_YELLOW = 103,
	CONSOLE_BG_BRIGHT_BLUE = 104,
	CONSOLE_BG_BRIGHT_MAGENTA = 105,
	CONSOLE_BG_BRIGHT_CYAN = 106,
	CONSOLE_BG_BRIGHT_WHITE = 107,
} console_bg_color_t;

typedef struct {
	console_fg_color_t foreground;
	console_bg_color_t background;
} console_color_t;

/* Console Color Configuration */
console_color_t getCurrentColors();
console_color_t getDefaultColors();
void setConsoleDefaults(console_color_t c);
void setConsoleForegroundDefault(console_fg_color_t c);
void setConsoleBackgroundDefault(console_bg_color_t c);
wallshell_error_t setForegroundColor(console_fg_color_t color);
wallshell_error_t setBackgroundColor(console_bg_color_t color);
wallshell_error_t setConsoleColors(console_color_t colors);

/* Stream configurations. */
typedef enum {
	WALLSHELL_INPUT,
	WALLSHELL_OUTPUT,
	WALLSHELL_ERROR
} wallshell_stream;

void setStream(wallshell_stream type, FILE* stream);

/* Cursors */
typedef enum {
	CONSOLE_CURSOR_LEFT = 0x4b,
	CONSOLE_CURSOR_RIGHT = 0x4d,
	CONSOLE_CURSOR_UP = 0x48,
	CONSOLE_CURSOR_DOWN = 0x50,
} console_cursor_t;

#ifndef CUSTOM_CURSOR_CONTROL
void wallshell_move_cursor(console_cursor_t direction);
	#define MOVE_CURSOR(direction) wallshell_move_cursor(direction);
#endif // CUSTOM_CURSOR_CONTROL

/* General operations */
wallshell_error_t registerCommand(const command_t c);
void deregisterCommand(const command_t c);
wallshell_error_t executeCommand(char* commandBuf);
wallshell_error_t terminalMain();

/* Utility functions */
void printGeneralHelp(help_entry_general_t* entry);
void printSpecificHelp(help_entry_specific_t* entry);
void setConsoleLocale();
void initializeDefaultStreams();
bool promptUser(const char* format, ...);
void setConsolePrefix(const char* newPrefix);
bool compareCommands(const command_t c1, const command_t c2);
#endif // COMMAND_HANDLER_H
