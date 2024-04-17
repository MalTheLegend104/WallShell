/**
 * @file wall_shell.h
 * @author MalTheLegend104
 * @brief Main header file for WallShell.
 *
 * C99 compliant command handler. Meant to be easily portable and highly configurable.
 *
 * @version v1.0
 * @copyright
 * Copyright 2024 MalTheLegend104
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/* Freestanding headers. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

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
#ifdef DISABLE_MALLOC
#error "Threaded support can't exist without malloc."
#endif

#ifndef CUSTOM_THREADS
#ifdef _WIN32
#include <Windows.h>
/**
 * @brief Wrapper around your system's mutex type.
 * @note `CRITICAL_SECTION` is replaced with your systems mutex type.
 */
typedef CRITICAL_SECTION ws_mutex_t;
/**
 * @brief Wrapper around your system's thread handle.
 * @note `DWORD` is replaced with your systems thread handle type.
 */
typedef DWORD ws_thread_id_t;

#else
#include <pthread.h>
/**
 * @brief Wrapper around your system's mutex type.
 * @note `pthread_mutex_t` is replaced with your systems mutex type.
 */
typedef pthread_mutex_t ws_mutex_t;
/**
 * @brief Wrapper around your system's thread hadnle.
 * @note `uint64_t` is replaced with your systems thread handle type.
 */
typedef uint64_t ws_thread_id_t;
#endif // _WIN32
#endif
/* Mutex */
void ws_lockMutex(ws_mutex_t* mut);
void ws_unlockMutex(ws_mutex_t* mut);
ws_mutex_t* ws_createMutex();
void ws_destroyMutex(ws_mutex_t* mut);

/* Thread ID */
ws_thread_id_t ws_getThreadID();

/* Atomic Bool */
typedef struct {
	bool b;
	ws_mutex_t* mut;
} ws_atomic_bool_t;

bool ws_getAtomicBool(ws_atomic_bool_t* ab);
void ws_setAtomicBool(ws_atomic_bool_t* ab, bool b);
ws_atomic_bool_t* ws_createAtomicBool(bool b);
void ws_destroyAtomicBool(ws_atomic_bool_t* ab);

void ws_sleep(size_t ms);

void ws_stopTerminal();

/* Thread names for logging */
#ifndef NO_WALLSHELL_LOGGING
void ws_setThreadName(char* name);
void ws_removeThreadName(const char* name);
void ws_printThreadID();
void ws_doPrintThreadID(bool b);
#endif // NO_WALLSHELL_LOGGING
#endif // THREADED_SUPPORT

#ifdef DISABLE_MALLOC
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
	WALLSHELL_WS_SETUP_ERROR
} ws_error_t;

typedef struct {
	int (*mainCommand)(int argc, char** argv);
	int (*helpCommand)(int argc, char** argv);
	const char* commandName;
	const char** aliases;
	size_t aliases_count;
} ws_command_t;

typedef struct {
	const char* commandName;
	const char* description;
	const char** commands;
	const int commands_count;
	const char** aliases;
	const int aliases_count;
} ws_help_entry_general_t;

typedef struct {
	const char* commandName;
	const char* description;
	const char** required;
	const int required_count;
	const char** optional;
	const int optional_count;
} ws_help_entry_specific_t;

typedef enum {
	WS_FG_DEFAULT = 0,
	WS_FG_BLACK = 30,
	WS_FG_RED = 31,
	WS_FG_GREEN = 32,
	WS_FG_YELLOW = 33,
	WS_FG_BLUE = 34,
	WS_FG_MAGENTA = 35,
	WS_FG_CYAN = 36,
	WS_FG_WHITE = 37,

	WS_FG_BRIGHT_BLACK = 90,
	WS_FG_BRIGHT_RED = 91,
	WS_FG_BRIGHT_GREEN = 92,
	WS_FG_BRIGHT_YELLOW = 93,
	WS_FG_BRIGHT_BLUE = 94,
	WS_FG_BRIGHT_MAGENTA = 95,
	WS_FG_BRIGHT_CYAN = 96,
	WS_FG_BRIGHT_WHITE = 97,
} ws_fg_color_t;

typedef enum {
	WS_BG_DEFAULT = 0,
	WS_BG_BLACK = 40,
	WS_BG_RED = 41,
	WS_BG_GREEN = 42,
	WS_BG_YELLOW = 43,
	WS_BG_BLUE = 44,
	WS_BG_MAGENTA = 45,
	WS_BG_CYAN = 46,
	WS_BG_WHITE = 47,

	WS_BG_BRIGHT_BLACK = 100,
	WS_BG_BRIGHT_RED = 101,
	WS_BG_BRIGHT_GREEN = 102,
	WS_BG_BRIGHT_YELLOW = 103,
	WS_BG_BRIGHT_BLUE = 104,
	WS_BG_BRIGHT_MAGENTA = 105,
	WS_BG_BRIGHT_CYAN = 106,
	WS_BG_BRIGHT_WHITE = 107,
} ws_bg_color_t;

typedef struct {
	ws_fg_color_t foreground;
	ws_bg_color_t background;
} ws_color_t;

/* Console Color Configuration */
ws_color_t ws_getCurrentColors();
ws_color_t ws_getDefaultColors();
void ws_setDefaultColors(ws_color_t c);
void ws_setForegroundDefault(ws_fg_color_t c);
void ws_setBackgroundDefault(ws_bg_color_t c);
ws_error_t ws_setForegroundColor(ws_fg_color_t color);
ws_error_t ws_setBackgroundColor(ws_bg_color_t color);
ws_error_t ws_setConsoleColors(ws_color_t colors);

/* Stream configurations. */
typedef enum {
	WALLSHELL_INPUT,  /* Input stream. Defaults to stdin. */
	WALLSHELL_OUTPUT, /* Ouput stream. Defaults to stdout. */
	WALLSHELL_ERROR   /* Error stream. Defaults to stderr. */
} ws_stream;

void ws_setStream(ws_stream type, FILE* stream);

/* Cursors */
typedef enum {
	WS_CURSOR_LEFT = 0x4b,
	WS_CURSOR_RIGHT = 0x4d,
	WS_CURSOR_UP = 0x48,
	WS_CURSOR_DOWN = 0x50,
} ws_cursor_t;

void ws_moveCursor(ws_cursor_t direction);
void ws_moveCursor_n(ws_cursor_t direction, size_t n);

/* General operations */
ws_error_t ws_registerCommand(const ws_command_t c);
void ws_deregisterCommand(const ws_command_t c);
ws_error_t ws_executeCommand(char* commandBuf);
ws_error_t ws_terminalMain();

/* Console Setup */
void ws_setAsciiDeleteAsBackspace(bool b);
void ws_setConsoleLocale();
void ws_setConsolePrefix(const char* newPrefix);
void ws_initializeDefaultStreams();

/* Utility functions */
void ws_printGeneralHelp(ws_help_entry_general_t* entry);
void ws_printSpecificHelp(ws_help_entry_specific_t* entry);
bool ws_promptUser(const char* format, ...);
bool ws_compareCommands(const ws_command_t c1, const ws_command_t c2);
void ws_cleanAll();

/* Logger */
#ifndef NO_WALLSHELL_LOGGING
typedef enum {
	WS_LOG,
	WS_DEBUG,
	WS_INFO,
	WS_WARN,
	WS_ERROR,
	WS_FATAL
} ws_logtype_t;

void ws_logger(ws_logtype_t type, const char* format, ...);
void ws_vlogger(ws_logtype_t type, const char* format, va_list args);
void ws_setLoggerColors(ws_logtype_t type, ws_fg_color_t fg, ws_bg_color_t bg);
#endif // NO_WALLSHELL_LOGGING

#endif // COMMAND_HANDLER_H