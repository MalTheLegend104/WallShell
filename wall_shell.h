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
	#ifndef COMMAND_LIMIT
		#define COMMAND_LIMIT 25
	#endif
	#ifndef MAX_ARGS
		#define MAX_ARGS 32
	#endif
#endif // DISABLE_MALLOC

// ------------------------------------------------------------------------------------------------
// If the user hasn't defined streams, set the defaults.
// ------------------------------------------------------------------------------------------------
#ifndef NO_STD_STREAMS

#endif // NO_STD_STREAMS

typedef enum {
	WALLSHELL_NO_ERROR = 0,
	WALLSHELL_OUT_OF_MEMORY,
	WALLSHELL_COMMAND_LIMIT_REACHED,
	WALLSHELL_OUT_STREAM_NOT_SET,
	WALLSHELL_CANT_SET_DEFAULT_TO_DEFAULT
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
	CONSOLE_FG_BLACK,
	CONSOLE_FG_BRIGHT_BLACK,
	CONSOLE_FG_WHITE,
	CONSOLE_FG_BRIGHT_WHITE,
	CONSOLE_FG_RED,
	CONSOLE_FG_BRIGHT_RED,
	CONSOLE_FG_GREEN,
	CONSOLE_FG_BRIGHT_GREEN,
	CONSOLE_FG_YELLOW,
	CONSOLE_FG_BRIGHT_YELLOW,
	CONSOLE_FG_CYAN,
	CONSOLE_FG_BRIGHT_CYAN,
	CONSOLE_FG_BLUE,
	CONSOLE_FG_BRIGHT_BLUE,
	CONSOLE_FG_MAGENTA,
	CONSOLE_FG_BRIGHT_MAGENTA,
} console_fg_color_t;

typedef enum {
	CONSOLE_BG_DEFAULT = 0,
	CONSOLE_BG_BLACK,
	CONSOLE_BG_BRIGHT_BLACK,
	CONSOLE_BG_WHITE,
	CONSOLE_BG_BRIGHT_WHITE,
	CONSOLE_BG_RED,
	CONSOLE_BG_BRIGHT_RED,
	CONSOLE_BG_GREEN,
	CONSOLE_BG_BRIGHT_GREEN,
	CONSOLE_BG_YELLOW,
	CONSOLE_BG_BRIGHT_YELLOW,
	CONSOLE_BG_CYAN,
	CONSOLE_BG_BRIGHT_CYAN,
	CONSOLE_BG_BLUE,
	CONSOLE_BG_BRIGHT_BLUE,
	CONSOLE_BG_MAGENTA,
	CONSOLE_BG_BRIGHT_MAGENTA
} console_bg_color_t;

typedef struct {
	console_fg_color_t foreground;
	console_bg_color_t background;
} console_color_t;

/* Console Color Configuration */
console_color_t getCurrentColors();
console_color_t getDefaultColors();
wallshell_error_t setConsoleDefaults(console_color_t c);
wallshell_error_t setConsoleForegroundDefault(console_fg_color_t c);
wallshell_error_t setConsoleBackgroundDefault(console_bg_color_t c);
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

/* General operations */
wallshell_error_t registerCommand(const command_t c);
void deregisterCommand(const command_t c);
void executeCommand(char* commandBuf);
wallshell_error_t terminalMain();

/* Utility functions */
void printGeneralHelp(help_entry_general_t* entry);
void printSpecificHelp(help_entry_specific_t* entry);
void setConsoleLocale();
void initalizeDefaultStreams();
		
bool compareCommands(const command_t c1, const command_t c2);
#endif // COMMAND_HANDLER_H
