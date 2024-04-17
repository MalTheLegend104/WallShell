/**
 * @file wall_shell.c
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
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

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// General header & compiler config
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#include "wall_shell.h"
#ifdef _WIN32
#include <Windows.h>
#endif

/* Disable unused parameter warnings. This only affects this file. */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(disable : 4100)
#endif

// Thank you  microsoft for making my life harder
#ifdef _MSC_VER
// Disables the warning for string.h "deprecated" functions
// We're in C99, and the stuff we're touching should either be manually mutex locked or single threaded anyway.
// Commands that are outside the scope of this file should be properly mutex locked if they touch things running in other threads.
#pragma warning(disable : 4996)
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Internal Utility Functions
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
 * @internal
 * @brief Internal function to concat a single char to the end of a string.
 * If the string is already at max length (including room for '\0') then the string is returned unchanged.
 *
 * @param string String to add the character to.
 * @param c Character to add to the string.
 * @param size Size of the entire buffer.
 * @return char* Same as the string passed through.
 */
char* ws_internal_strcat_c(char* string, char c, size_t size) {
	// Find the current length of the string
	size_t current_length = strlen(string);

	// If adding something would make the string too long, we return unchanged
	if (current_length + 1 >= size) return string;

	// Add the character to the end of the string
	string[current_length] = c;
	string[current_length + 1] = '\0';

	return string;
}

/**
 * @internal
 * @brief Internal function to insert a single char into a string.
 * If the string is already at max length (including room for '\0') then the string is returned unchanged.
 *
 * @param string String to add the character to.
 * @param buf_size Size of the entire buffer.
 * @param c Character to add to the string.
 * @param position Position that the character is inserted into. It should be index + 1
 * @return char* Same as the string passed through.
 */
char* ws_internal_insert_c(char* string, size_t buf_size, char c, size_t position) {
	size_t current_length = strlen(string);
	if (current_length + 1 >= buf_size) return string;

	for (size_t i = current_length; i > position - 1; i--) {
		string[i] = string[i - 1];
	}
	string[position - 1] = c;

	return string;
}

/**
 * @internal
 * @brief Checks if a string starts with another string.
 *
 * As far as I know, there is nothing in libc to do this, although I might've just overlooked something.
 * Regardless, this is a pretty simple function, just a little bit of pointer magic.
 * It's an internal function, but you can easily use it using either extern or adding the declaration to `wallshell_config.h`.
 *
 * @param str String to check
 * @param prefix Thing to check that the other starts with.
 * @return true If the string starts with prefix.
 * @return false If the string does not start with prefix.
 */
bool ws_internal_startsWith(const char* str, const char* prefix) {
	while (*prefix && *str) {
		if (*prefix != *str) {
			return false;
		}
		prefix++;
		str++;
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Streams
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
FILE* ws_out_stream = NULL;
FILE* ws_err_stream = NULL;
FILE* ws_in_stream = NULL;

/**
 * @brief Sets the stream to the provided one.
 * @param type Type of stream to change.
 * @param stream Stream you wish to change it to.
 */
void ws_setStream(ws_stream type, FILE* stream) {
	switch (type) {
		case WALLSHELL_INPUT: ws_in_stream = stream;
			break;
		case WALLSHELL_OUTPUT: ws_out_stream = stream;
			break;
		case WALLSHELL_ERROR: ws_err_stream = stream;
			break;
		default: break;
	}
}

/**
 * @brief Initialize all streams to their defaults. All default to their std-versions. (stdout, stderr, stdin)
 */
void ws_initializeDefaultStreams() {
	ws_setStream(WALLSHELL_INPUT, stdin);
	ws_setStream(WALLSHELL_ERROR, stderr);
	ws_setStream(WALLSHELL_OUTPUT, stdout);
}

/**
 * @internal
 * @brief Internal function to reset streams to their default state.
 */
void ws_internal_cleanStreams() {
	ws_out_stream = NULL;
	ws_err_stream = NULL;
	ws_in_stream = NULL;
}

#ifndef CLEAR_ROW
#define CLEAR_ROW fprintf(ws_out_stream, "\033[M");
#endif // CLEAR_ROW

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Custom Console Setup
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// For some systems (mostly POSIX), backspace gets sent as ascii delete rather than \b
bool backspace_as_ascii_delete = false;
#ifndef CUSTOM_WS_SETUP
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
struct termios old_settings, new_settings;
#endif // _WIN32
/**
 * @brief Sets the console mode to the state that WallShell needs.
 *
 * This includes enabling virtual terminal input/output, disabling input buffering, and disabling echo input.
 *
 * @return WALLSHELL_WS_SETUP_ERROR if unsuccessful, WALLSHELL_NO_ERROR otherwise.
 */
ws_error_t setConsoleMode() {
#ifdef _WIN32
	// Set output mode to handle virtual terminal sequences
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return WALLSHELL_WS_SETUP_ERROR;

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	if (hIn == INVALID_HANDLE_VALUE) return WALLSHELL_WS_SETUP_ERROR;

	DWORD dwOriginalOutMode = 0;
	DWORD dwOriginalInMode = 0;
	if (!GetConsoleMode(hOut, &dwOriginalOutMode)) return WALLSHELL_WS_SETUP_ERROR;
	if (!GetConsoleMode(hIn, &dwOriginalInMode)) return WALLSHELL_WS_SETUP_ERROR;

	DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

	DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	if (!SetConsoleMode(hOut, dwOutMode)) {
		// we failed to set both modes, try to step down mode gracefully.
		dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
		if (!SetConsoleMode(hOut, dwOutMode)) {
			// Failed to set any VT mode, can't do anything here.
			return WALLSHELL_WS_SETUP_ERROR;
		}
	}

	DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
	if (!SetConsoleMode(hIn, dwInMode)) {
		// Failed to set VT input mode, can't do anything here.
		return WALLSHELL_WS_SETUP_ERROR;
	}

	// Disable a few features that make a command handler hard to make
	DWORD inMode = 0;
	if (!GetConsoleMode(hIn, &inMode)) { return WALLSHELL_WS_SETUP_ERROR; }
	inMode &= ~ENABLE_LINE_INPUT;
	inMode &= ~ENABLE_PROCESSED_INPUT;
	inMode &= ~ENABLE_ECHO_INPUT;
	if (!SetConsoleMode(hIn, inMode)) { return WALLSHELL_WS_SETUP_ERROR; }
	fprintf(ws_out_stream, "\033=\033[?1h");
#else
	// POSIX terminals support
	tcgetattr(STDIN_FILENO, &old_settings);
	new_settings = old_settings;
	new_settings.c_lflag &= ~(ICANON | ECHO | IEXTEN);

	// posix terminals send ASCII delete instead of backspace for some godforsaken reason
	backspace_as_ascii_delete = true;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
#endif // _WIN32
	return WALLSHELL_NO_ERROR;
}

/**
 * @brief Resets the console state. This is called automatically by ws_cleanAll().
 */
void ws_resetConsoleState() {
#ifndef _WIN32
	// Restore original terminal settings
	tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
#endif // _WIN32
}

#ifdef _WIN32
#include <conio.h>
#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
#define ws_get_char_blocking(stream) _getch()
#else
#define PRINTING_NEEDS_FLUSH
#define SET_TERMINAL_LOCALE
#define ws_get_char_blocking(stream) getchar()
#endif // _WIN32

/**
 * @internal
 * @brief Non blocking version of getc().
 *
 * @return int Same as getc() and -2 if no new characters in the stream.
 */
int ws_internal_getCharNonBlocking() {
#ifdef _WIN32
	if (_kbhit()) {
		return _getch();
	} else {
		return -2;
	}
#else
	fd_set set;
	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);
	struct timeval timeout = { 0, 0 }; // Immediate return
	int ready = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

	if (ready == -1) {
		perror("select");
		exit(EXIT_FAILURE);
	} else if (ready > 0) {
		return getchar();
	} else {
		return -2;
	}
#endif
}
#define ws_get_char(stream) ws_internal_getCharNonBlocking()
#endif // CUSTOM_WS_SETUP

// To aid portability, we allow the user to set backspace_as_ascii_delete
/**
 * @brief Some consoles send backspace as ASCII delete (0x7f) instead of '\\b'.
 *
 * If your system does this, set this to true. This only needs to be done if CUSTOM_WS_SETUP is defined.
 * For POSIX this is typically true, for Windows this is false.
 *
 * @param b Bool to set backspace_as_ascii_delete to.
 */
void ws_setAsciiDeleteAsBackspace(bool b) { backspace_as_ascii_delete = b; }

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Default console color output.
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifndef CUSTOM_WS_COLORS
#ifndef SET_TERMINAL_LOCALE
#ifdef _WIN32
#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define SET_TERMINAL_LOCALE
#endif
#endif
/* Modern windows is supposed to support these escape codes, older windows versions use SetConsoleTextAttribute */
/* https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences */
#define RESET_CONSOLE fprintf(ws_out_stream, "\033[0m")

/**
 * @internal
 * @brief Changes the console color using virtual terminal sequences.
 *
 * @param fg Foreground color
 * @param bg Background color
 */
void ws_internal_changeConsoleColor(ws_fg_color_t fg, ws_bg_color_t bg) {
	if (fg == WS_FG_DEFAULT || bg == WS_BG_DEFAULT) {
		RESET_CONSOLE;
	}

	if (fg == WS_FG_DEFAULT && bg != WS_BG_DEFAULT) {
		fprintf(ws_out_stream, "\033[%dm", bg);
	} else if (fg != WS_FG_DEFAULT && bg == WS_BG_DEFAULT) {
		fprintf(ws_out_stream, "\033[%dm", fg);
	} else {
		fprintf(ws_out_stream, "\033[%d;%dm", fg, bg);
	}
}

#define SET_WS_COLORS(a, b) ws_internal_changeConsoleColor(a, b);

#endif // CUSTOM_WS_COLORS

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Console Colors
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifdef THREADED_SUPPORT
ws_mutex_t* color_mutex = NULL;
void ws_internal_color_mutex_check() {
	if (!color_mutex) color_mutex = ws_createMutex();
	// We don't really care if it's NULL.
}
#define COLOR_MUTEX_CHECK ws_internal_color_mutex_check()
#define LOCK_COLOR_MUTEX ws_lockMutex(color_mutex)
#define UNLOCK_COLOR_MUTEX ws_unlockMutex(color_mutex)

#else
#define COLOR_MUTEX_CHECK
#define LOCK_COLOR_MUTEX
#define UNLOCK_COLOR_MUTEX
#endif
ws_color_t default_colors = { WS_FG_DEFAULT, WS_BG_DEFAULT };
ws_color_t current_colors = { WS_FG_DEFAULT, WS_BG_DEFAULT };

/**
 * @internal
 * @brief Resets all color variables to their defualt state.
 */
void ws_internal_cleanColors() {
#ifdef THREADED_SUPPORT
	if (color_mutex) ws_destroyMutex(color_mutex);
	color_mutex = NULL;
#endif // THREADED_SUPPORT
	default_colors.foreground = WS_FG_DEFAULT;
	default_colors.background = WS_BG_DEFAULT;
	current_colors.foreground = WS_FG_DEFAULT;
	current_colors.background = WS_BG_DEFAULT;
}

/**
 * @internal
 * @brief Updates the current colors.
 *
 * @return ws_error_t WALLSHELL_OUT_STREAM_NOT_SET if the output stream hasn't been set yet. WALLSHELL_NO_ERROR otherwise.
 */
ws_error_t ws_internal_updateColors() {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	if (!ws_out_stream) return WALLSHELL_OUT_STREAM_NOT_SET;
	if (current_colors.foreground == WS_FG_DEFAULT) {
		current_colors.foreground = default_colors.foreground;
	}
	if (current_colors.background == WS_BG_DEFAULT) {
		current_colors.background = default_colors.background;
	}
	SET_WS_COLORS(current_colors.foreground, current_colors.background);
	UNLOCK_COLOR_MUTEX;
	return WALLSHELL_NO_ERROR;
}

/**
 * @brief Set the default foreground color.
 * @param c Color to set the default to.
 */
void ws_setForegroundDefault(ws_fg_color_t c) {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	default_colors.foreground = c;
	UNLOCK_COLOR_MUTEX;
}

/**
 * @brief Set the default background color.
 * @param c Color to set the default to.
 */
void ws_setBackgroundDefault(ws_bg_color_t c) {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	default_colors.background = c;
	UNLOCK_COLOR_MUTEX;
}

/**
 * @brief Set the default colors to the provided ones.
 * @param c Colors to set the defaults to.
 */
void ws_setDefaultColors(ws_color_t c) {
	ws_setForegroundDefault(c.foreground);
	ws_setBackgroundDefault(c.background);
}

/**
 * @brief Get the current console colors.
 * @return  The ws_color_t of the current colors.
 */
ws_color_t ws_getCurrentColors() { return current_colors; }

/**
 * @brief Get the current default colors.
 * @return The ws_color_t of the default colors.
 */
ws_color_t ws_getDefaultColors() { return default_colors; }

/**
 * @brief Set the background and foreground colors to the provided ones.
 * @param colors Colors to set the background and foreground to.
 * @return Can return WALLSHELL_OUT_STREAM_NOT_SET if the stream hasn't be initialized.
 */
ws_error_t ws_setConsoleColors(ws_color_t colors) {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	current_colors.foreground = colors.foreground;
	current_colors.background = colors.background;
	UNLOCK_COLOR_MUTEX;
	return ws_internal_updateColors();
}

/**
 * @brief Sets the foreground color to the provided color.
 * @param color Color to set the foreground to.
 * @return Can return WALLSHELL_OUT_STREAM_NOT_SET if the stream hasn't be initialized.
 */
ws_error_t ws_setForegroundColor(ws_fg_color_t color) {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	current_colors.foreground = color;
	UNLOCK_COLOR_MUTEX;
	return ws_internal_updateColors();
}

/**
 * @brief Sets the background color to the provided color.
 * @param color Color to set the background to.
 * @return Can return WALLSHELL_OUT_STREAM_NOT_SET if the stream hasn't be initialized.
 */
ws_error_t ws_setBackgroundColor(ws_bg_color_t color) {
	COLOR_MUTEX_CHECK;
	LOCK_COLOR_MUTEX;
	current_colors.background = color;
	UNLOCK_COLOR_MUTEX;
	return ws_internal_updateColors();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Threads
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifdef THREADED_SUPPORT
#ifndef CUSTOM_THREADS

#ifdef _WIN32
/**
 * @brief Locks the provided mutex.
 * @param mut Mutex to be locked.
 */
void ws_lockMutex(ws_mutex_t* mut) { EnterCriticalSection(mut); }
/**
 * @brief Unlocks the provided mutex.
 * @param mut Mutex to be unlocked.
 */
void ws_unlockMutex(ws_mutex_t* mut) { LeaveCriticalSection(mut); }

/**
 * @brief Create a mutex.
 * @return Pointer to a mutex object if successful, NULL otherwise.
 */
ws_mutex_t* ws_createMutex() {
	ws_mutex_t* mut = (ws_mutex_t*) malloc(sizeof(ws_mutex_t));
	if (mut == NULL) return NULL;
	InitializeCriticalSection(mut);
	return mut;
}

/**
 * @brief Destroys the provided mutex.
 * @param mut mutex to be destroyed.
 */
void ws_destroyMutex(ws_mutex_t* mut) {
	if (!mut) return;
	ws_lockMutex(mut);
	DeleteCriticalSection(mut);
	free(mut);
}

/**
 * @brief Gets the threadID of the calling thread.
 * @return ws_thread_id_t relating to the calling thread.
 */
ws_thread_id_t ws_getThreadID() { return GetCurrentThreadId(); }

/**
 * @internal
 * @brief Prints the threadID related to the calling thread.
 *
 * @param stream Output stream for fprintf.
 */
void ws_internal_printThreadID(FILE* stream) { fprintf(stream, "%lu", ws_getThreadID()); }

/**
 * @brief Sleep function wrapper.
 * @param ms Sleep time in milliseconds.
 */
void ws_sleep(size_t ms) {
	Sleep(ms);
}
#else

/**
 * @brief Locks the provided mutex.
 * @param mut Mutex to be locked.
 */
void ws_lockMutex(ws_mutex_t* mut) { pthread_mutex_lock(mut); }

/**
 * @brief Unlocks the provided mutex.
 * @param mut Mutex to be unlocked.
 */
void ws_unlockMutex(ws_mutex_t* mut) { pthread_mutex_unlock(mut); }

/**
 * @brief Create a mutex.
 * @return Pointer to a mutex object if successful, NULL otherwise.
 */
ws_mutex_t* ws_createMutex() {
	ws_mutex_t* mut = (ws_mutex_t*) malloc(sizeof(ws_mutex_t));
	if (mut == NULL) return NULL;
	pthread_mutex_init(mut, NULL);
	return mut;
}

/**
 * @brief Destroys the provided mutex.
 * @param mut mutex to be destroyed.
 */
void ws_destroyMutex(ws_mutex_t* mut) {
	ws_lockMutex(mut);
	pthread_mutex_destroy(mut);
	free(mut);
}
/**
 * @brief Gets the threadID of the calling thread.
 * @return ws_thread_id_t relating to the calling thread.
 */
ws_thread_id_t ws_getThreadID() { return pthread_self(); }
/**
 * @internal
 * @brief Prints the threadID related to the calling thread.
 *
 * @param stream Output stream for fprintf.
 */
void ws_internal_printThreadID(FILE* stream) { fprintf(stream, "%zu", ws_getThreadID()); }

/**
 * @brief Sleep function wrapper.
 * @param ms Sleep time in milliseconds.
 */
void ws_sleep(size_t ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000; // Convert remaining milliseconds to nanoseconds

	nanosleep(&ts, NULL);
}
#endif // _WIN32
#endif // CUSTOM_THREADS

/**
 * @brief Gets the value stored by the atomic bool.
 * @param ab Atomic bool object pointer.
 * @return False if bool does not exist, otherwise the value contained by the bool.
 */
bool ws_getAtomicBool(ws_atomic_bool_t* ab) {
	if (!ab) return false;
	bool ret;
	ws_lockMutex(ab->mut);
	ret = ab->b;
	ws_unlockMutex(ab->mut);
	return ret;
}

/**
 * @brief Sets the value of the provided atomic bool.
 * @param ab Atomic bool object.
 * @param b Value to set the bool to.
 */
void ws_setAtomicBool(ws_atomic_bool_t* ab, bool b) {
	if (!ab) return;
	ws_lockMutex(ab->mut);
	ab->b = b;
	ws_unlockMutex(ab->mut);
}

/**
 * @brief Creates an atomic bool.
 * @param b Initial value held by the bool.
 * @return NULL if it couldn't be created, pointer to the object otherwise.
 */
ws_atomic_bool_t* ws_createAtomicBool(bool b) {
	ws_atomic_bool_t* ab = (ws_atomic_bool_t*) malloc(sizeof(ws_atomic_bool_t));
	if (ab == NULL) return NULL;
	ab->b = b;
	ab->mut = ws_createMutex();
	return ab;
}

/**
 * @brief Destroys the provided atomic bool.
 * @param ab Atomic bool to destroy.
 */
void ws_destroyAtomicBool(ws_atomic_bool_t* ab) {
	ws_lockMutex(ab->mut);
	ws_destroyMutex(ab->mut);
	free(ab);
}

#endif // THREADED_SUPPORT

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Logging Functions
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifndef NO_WALLSHELL_LOGGING
#ifdef THREADED_SUPPORT
ws_mutex_t* logging_mutex = NULL;
#define LOCK_LOGGING_MUTEX ws_lockMutex(logging_mutex)
#define UNLOCK_LOGGING_MUTEX ws_unlockMutex(logging_mutex)

typedef struct {
	char* name;
	ws_thread_id_t id;
} ws_thread_map_t;

ws_thread_map_t* thread_map = NULL;
size_t thread_map_size = 0;
size_t thread_map_current = 0;

ws_mutex_t* thread_map_mut = NULL;
/**
 * @brief Sets the name of the calling thread. This will only be printed if threadID is true.
 * @param name Name of the thread.
 */
void ws_setThreadName(char* name) {
	if (!thread_map_mut) {
		thread_map_mut = ws_createMutex();
		if (!thread_map_mut) return;
	}
	ws_lockMutex(thread_map_mut);
	if (!thread_map) {
		thread_map = (ws_thread_map_t*) calloc(1, sizeof(ws_thread_map_t));
		if (!thread_map) return;
		thread_map_size++;
	}
	if (thread_map_size - 1 < thread_map_current) {
		ws_thread_map_t* temp = realloc(thread_map, (thread_map_size + 1) * sizeof(ws_thread_map_t));
		if (!temp) return;
		thread_map = temp;
		thread_map_size++;
	}
	char* thread_name = calloc(strlen(name), sizeof(char));
	if (!thread_name) return;
	strcpy(thread_name, name);

	thread_map[thread_map_current].name = thread_name;
	thread_map[thread_map_current].id = ws_getThreadID();

	thread_map_current++;
	ws_unlockMutex(thread_map_mut);
}

/**
 * @brief Removes the name of the provided thread.
 * @param name Name of the thread.
 */
void ws_removeThreadName(const char* name) {
	if (!thread_map_mut) {
		thread_map_mut = ws_createMutex();
		if (!thread_map_mut) return;
	}
	ws_lockMutex(thread_map_mut);

	if (!thread_map) return;
	if (thread_map_current == 0) return;
	if (thread_map_size == 0) return; // This should be impossible.

	for (int i = 0; i < thread_map_current; i++) {
		if (strcmp(thread_map[thread_map_current].name, name) == 0) {
			free(thread_map[thread_map_current].name);
			for (int j = i; j < thread_map_current - 1; j++) {
				thread_map[j].id = thread_map[j + 1].id;
				thread_map[j].name = thread_map[j + 1].name;
			}
			ws_unlockMutex(thread_map_mut);
			return;
		}
	}

	ws_unlockMutex(thread_map_mut);
}

/**
 * @brief Prints the threadID of the calling thread.
 */
void ws_printThreadID() {
	if (!thread_map_mut) {
		thread_map_mut = ws_createMutex();
		if (!thread_map_mut) return;
	}
	ws_lockMutex(thread_map_mut);
	ws_thread_id_t cur = ws_getThreadID();
	for (int i = 0; i < thread_map_current; i++) {
		if (thread_map[i].id == cur) {
			fprintf(ws_out_stream, "%s", thread_map[i].name);
			goto exit;
		}
	}
	ws_internal_printThreadID(ws_out_stream);
exit:
	ws_unlockMutex(thread_map_mut);
}

bool printThreadID = true;
/**
 * @brief Set print threadID, which prints the threadID of function calling `ws_logger`. Defaults to on.
 * @param b True to turn on, false to turn off.
 */
void ws_doPrintThreadID(bool b) { printThreadID = b; }
#else
#define LOCK_LOGGING_MUTEX
#define UNLOCK_LOGGING_MUTEX
#endif

/**
 * @internal
 * @brief Checks the logging mutex and makes sure the output stream is set.
 */
void ws_internal_logging_check() {
#ifdef THREADED_SUPPORT
	if (!logging_mutex) logging_mutex = ws_createMutex(); // We don't really care if it's NULL.
#endif // THREADED_SUPPORT
	// Make sure we have an out stream.
	if (!ws_out_stream) ws_setStream(WALLSHELL_OUTPUT, stdout);
}

#define LOGGING_CHECK ws_internal_logging_check()

ws_color_t log_colors = { WS_FG_WHITE, WS_BG_DEFAULT };
ws_color_t debug_colors = { WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT };
ws_color_t info_colors = { WS_FG_BRIGHT_CYAN, WS_BG_DEFAULT };
ws_color_t warn_colors = { WS_FG_BRIGHT_YELLOW, WS_BG_DEFAULT };
ws_color_t error_colors = { WS_FG_BRIGHT_RED, WS_BG_DEFAULT };
ws_color_t fatal_colors = { WS_FG_RED, WS_BG_DEFAULT };

/**
 * @internal
 * @brief Logger [LOG] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_vlogf(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(log_colors);
	fprintf(ws_out_stream, "[LOG]  ");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");
	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @internal
 * @brief Logger [DEBUG] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_vdebugf(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(debug_colors);
	fprintf(ws_out_stream, "[DEBUG]");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");
	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @internal
 * @brief Logger [INFO] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_vinfof(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(info_colors);
	fprintf(ws_out_stream, "[INFO] ");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");

	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @internal
 * @brief Logger [WARN] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_vwarnf(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(warn_colors);
	fprintf(ws_out_stream, "[WARN] ");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");

	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @internal
 * @brief Logger [ERROR] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_verrorf(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(error_colors);
	fprintf(ws_out_stream, "[ERROR]");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");

	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @internal
 * @brief Logger [FATAL] function
 *
 * @param format printf format string
 * @param args vprintf va_list
 */
void ws_vfatalf(const char* format, va_list args) {
	LOGGING_CHECK;
	LOCK_LOGGING_MUTEX;
	ws_color_t current = ws_getCurrentColors();
	ws_setConsoleColors(fatal_colors);
	fprintf(ws_out_stream, "[FATAL]");

#ifdef THREADED_SUPPORT
	if (printThreadID) {
		fprintf(ws_out_stream, "[");
		ws_printThreadID();
		fprintf(ws_out_stream, "]");
	}
#endif
	fprintf(ws_out_stream, " ");
	vfprintf(ws_out_stream, format, args);
	fprintf(ws_out_stream, "\n");

	ws_setConsoleColors(current);
	UNLOCK_LOGGING_MUTEX;
}

/**
 * @brief Logger function for WallShell. vprintf like formatting, automatically adds a newline.
 * @param type Type of logging.
 * @param format printf style formatting string.
 * @param args va_list of arguments.
 */
void ws_vlogger(ws_logtype_t type, const char* format, va_list args) {
	switch (type) {
		case WS_LOG: {
				ws_vlogf(format, args);
				break;
			}
		case WS_DEBUG: {
				ws_vdebugf(format, args);
				break;
			}
		case WS_INFO: {
				ws_vinfof(format, args);
				break;
			}
		case WS_WARN: {
				ws_vwarnf(format, args);
				break;
			}
		case WS_ERROR: {
				ws_verrorf(format, args);
				break;
			}
		case WS_FATAL: {
				ws_vfatalf(format, args);
				break;
			}
		default: {
				vfprintf(ws_out_stream, format, args);
			}
	}
}

/**
 * @brief Logger function for WallShell. Printf like formatting, automatically adds a newline.
 * @param type Type of logging.
 * @param format Printf style formatting string.
 * @param ... Printf style formatting arguments.
 */
void ws_logger(ws_logtype_t type, const char* format, ...) {
	va_list args;
	va_start(args, format);
	ws_vlogger(type, format, args);
	va_end(args);
}

/**
 * @brief Set the logger colors for the specified log type.
 * @param type Type of logging.
 * @param fg Foreground color.
 * @param bg Background color.
 */
void ws_setLoggerColors(ws_logtype_t type, ws_fg_color_t fg, ws_bg_color_t bg) {
	switch (type) {
		case WS_LOG: {
				log_colors.foreground = fg;
				log_colors.background = bg;
				break;
			}
		case WS_INFO: {
				warn_colors.foreground = fg;
				warn_colors.background = bg;
				break;
			}
		case WS_DEBUG: {
				debug_colors.foreground = fg;
				debug_colors.background = bg;
				break;
			}
		case WS_WARN: {
				warn_colors.foreground = fg;
				warn_colors.background = bg;
				break;
			}
		case WS_ERROR: {
				error_colors.foreground = fg;
				error_colors.background = bg;
				break;
			}
		case WS_FATAL: {
				fatal_colors.foreground = fg;
				fatal_colors.background = bg;
				break;
			}
		default: break;
	}
}

/**
 * @internal
 * @brief Resets all logger variables. Resets color, mutex, etc.
 */
void ws_internal_cleanLogger() {
#ifdef THREADED_SUPPORT
	if (logging_mutex) ws_destroyMutex(logging_mutex);
	if (thread_map_mut) ws_destroyMutex(thread_map_mut);
	logging_mutex = NULL;
	printThreadID = true;
	if (thread_map) {
		for (int i = 0; i < thread_map_current; i++) {
			free(thread_map[i].name);
		}
		free(thread_map);
	}
	thread_map_size = 0;
	thread_map_current = 0;
	thread_map = NULL;
#endif
	log_colors = (ws_color_t){ WS_FG_WHITE, WS_BG_DEFAULT };
	debug_colors = (ws_color_t){ WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT };
	info_colors = (ws_color_t){ WS_FG_BRIGHT_CYAN, WS_BG_DEFAULT };
	warn_colors = (ws_color_t){ WS_FG_BRIGHT_YELLOW, WS_BG_DEFAULT };
	error_colors = (ws_color_t){ WS_FG_BRIGHT_RED, WS_BG_DEFAULT };
	fatal_colors = (ws_color_t){ WS_FG_RED, WS_BG_DEFAULT };
}

#endif // NO_WALLSHELL_LOGGING

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Register Command & Internal Commands
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

#ifdef DISABLE_MALLOC
command_t commands[COMMAND_LIMIT];
size_t command_size = COMMAND_LIMIT;
#else
ws_command_t* commands;
size_t command_size = 0;
#endif

size_t current_command_spot = 0;

char previousCommands[PREVIOUS_BUF_SIZE][MAX_COMMAND_BUF];
size_t previous_commands_size;

/**
 * @internal
 * @brief Resets all command variables. Resets command list, previousCommands, etc.
 */
void ws_internal_cleanCommands() {
#ifndef DISABLE_MALLOC
	free(commands);
	commands = NULL;
	command_size = 0;
#else
	for (int i = 0; i < current_command_spot; i++) {
		commands[current_command_spot].commandName = NULL;
		commands[current_command_spot].aliases = NULL;
		commands[current_command_spot].aliases_count = 0;
		commands[current_command_spot].helpCommand = NULL;
		commands[current_command_spot].mainCommand = NULL;
	}
	command_size = COMMAND_LIMIT;
#endif // DISABLE_MALLOC
	current_command_spot = 0;

	for (int i = 0; i < PREVIOUS_BUF_SIZE; i++) {
		memset(previousCommands, 0, MAX_COMMAND_BUF);
	}
	previous_commands_size = 0;
}

/**
 * @brief Register the command to the command handler.
 * @param c Command to be registered.
 * @return Can return WALLSHELL_COMMAND_LIMIT_REACHED if DISABLE_MALLOC is defined, and WALLSHELL_OUT_OF_MEMORY if not.
 */
ws_error_t ws_registerCommand(const ws_command_t c) {
#ifdef DISABLE_MALLOC
	if (current_command_spot != COMMAND_LIMIT) {
		commands[current_command_spot] = c;
		current_command_spot++;
	} else {
		return WALLSHELL_COMMAND_LIMIT_REACHED;
	}
#else
	if (!commands) {
		commands = malloc(sizeof(ws_command_t));
		command_size = 1;
	} else if (current_command_spot >= command_size) {
		bool was_one = false;
		if (command_size == 1) {
			was_one = true;
			command_size++;
		}
		// realloc invalidates the old pointer on call, but leaves it alone if it cant find the memory.
		// If this function returns with an out of memory error, the shell is still usable.
		ws_command_t* new_ptr = realloc(commands, (size_t) ((double) command_size * sizeof(ws_command_t) * 1.5));
		if (!new_ptr) {
			if (was_one)
				command_size--;
			return WALLSHELL_OUT_OF_MEMORY;
		} else {
			commands = new_ptr;
		}
		command_size = (size_t) ((double) command_size * 1.5);
	}
	//memcpy(commands[current_command_spot], &c, sizeof(command_t));
	commands[current_command_spot] = c;
	current_command_spot++;
#endif
	return WALLSHELL_NO_ERROR;
}

/**
 * @brief Deregister the provided command.
 * @param c Command to be deregistered. If it doesn't exist (not already registered), nothing happens.
 */
void ws_deregisterCommand(const ws_command_t c) {
	for (int i = 0; i < current_command_spot; i++) {
		if (ws_compareCommands(commands[i], c)) {
			for (int j = i; j < current_command_spot - 1; j++) {
				// Nothing is allocated through malloc. If something is, it's on the user to free it either before/after calling this.
				commands[j].commandName = commands[j + 1].commandName;
				commands[j].aliases = commands[j + 1].aliases;
				commands[j].aliases_count = commands[j + 1].aliases_count;
				commands[j].helpCommand = commands[j + 1].helpCommand;
				commands[j].mainCommand = commands[j + 1].mainCommand;
			}
			return;
		}
	}
}

/* Internal clear command */
const char* clear_aliases[] = { "clr", "cls" };
/**
 * @internal
 * @brief Clear function help command
 */
int clearHelp(int argc, char** argv) {
	ws_help_entry_general_t entry = {
			"Clear",
			"Clears the screen",
			NULL,
			0,
			clear_aliases,
			2
	};
	ws_printGeneralHelp(&entry);
	return 0;
}

/**
 * @internal
 * @brief Clear function main command
 */
int clearMain(int argc, char** argv) {
#ifdef _WIN32
	// Windows being windows, some escape characters don't work normally in like 2/3 of the terminals
	// This is especially evident in things spawned by AllocConsole()
	system("cls");
#else
	// Unix is much nicer
	fprintf(ws_out_stream, "\033c");
#endif
	// Sometimes clearing the screen results in the colors getting reset.
	ws_internal_updateColors();
	return 0;
}

/* Internal help command */
/**
 * @internal
 * @brief Help function search command
 */
void helpSearch(char* str) {
	ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
	fprintf(ws_out_stream, "List of commands starting with \"%s\": (A) indicates an alias.\n", str);
	ws_setConsoleColors(ws_getDefaultColors());
	for (int i = 0; i < current_command_spot; i++) {
		ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT });
		if (commands[i].commandName && ws_internal_startsWith(commands[i].commandName, str)) {
			fprintf(ws_out_stream, "\t%s\n", commands[i].commandName);
		}

		// Check aliases for a match
		for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
			if (commands[i].aliases[alias_idx] && ws_internal_startsWith(commands[i].aliases[alias_idx], str)) {
				fprintf(ws_out_stream, "\t%s (A)\n", commands[i].aliases[alias_idx]);
			}
		}
		ws_setConsoleColors(ws_getDefaultColors());
	}
}

/**
 * @internal
 * @brief Help function help command
 */
int helpHelp(int argc, char** argv) {
	const char* optional[] = {
			"-s <string> -> Lists all commands and aliases that start with <string>."
	};
	ws_help_entry_specific_t entry = {
			"Help",
			"The help menu.",
			NULL,
			0,
			optional,
			1
	};
	ws_printSpecificHelp(&entry);
	return 0;
}

/**
 * @internal
 * @brief Help function main command
 */
int helpMain(int argc, char** argv) {
	// Make sure there's more than one argument.
	if (argc > 1) {
		// remove help from argv
		// Shift all pointers one position to the left
		for (int i = 1; i < argc; i++) {
			argv[i - 1] = argv[i];
		}
		argv[argc - 1] = NULL;

		// Update the size of the array
		argc--;
		// Check for more args
		if (argc >= 1) {
			if ((strcmp(argv[0], "-s") == 0) || (strcmp(argv[0], "-search") == 0)) {
				if (argc == 1) {
					ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "Search flag must be followed by an argument.\n");
				} else {
					helpSearch(argv[1]);
					return 0;
				}
			}
		}

		// Find the command
		for (int i = 0; i < current_command_spot; i++) {
			// Check the normal command name
			if (commands[i].commandName && strcmp(commands[i].commandName, argv[0]) == 0) {
				// No help function for command.
				if (!commands[i].helpCommand) {
					ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "Command \"%s\" does not have a help function.\n", argv[0]);
					ws_setConsoleColors(ws_getDefaultColors());
					return 0;
				}

				// Execute the help command associated with the matched command
				int result = commands[i].helpCommand(argc, argv);
				if (result != 0) {
					// If the command function returns a non-zero value, it may indicate an error
					ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "Command exited with code: %d\n", result);
					ws_setConsoleColors(ws_getDefaultColors());
				}
				return 0;
			}

			// Check aliases for a match
			for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
				if (commands[i].aliases[alias_idx] && strcmp(commands[i].aliases[alias_idx], argv[0]) == 0) {
					// No help function for command.
					if (!commands[i].helpCommand) {
						ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
						fprintf(ws_out_stream, "Command \"%s\" does not have a help function.\n", argv[0]);
						ws_setConsoleColors(ws_getDefaultColors());
						return 0;
					}

					// Execute the help command associated with the matched alias
					int result = commands[i].helpCommand(argc, argv);
					if (result != 0) {
						// If the command function returns a non-zero value, it may indicate an error
						ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
						fprintf(ws_out_stream, "Command exited with code: %d\n", result);
						ws_setConsoleColors(ws_getDefaultColors());
					}
					return 0;
				}
			}
		}
		// If the command is not found in the registered commands or their aliases
		ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "Help command not found for: %s\n", argv[0]);
	} else {
		fprintf(ws_out_stream, "\n");
		ws_setConsoleColors((ws_color_t) { WS_FG_CYAN, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "To get more info about a command, run `help <command_name>`\n");
		ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "All commands:\n");

		ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT });
		// List all available commands
		for (int i = 0; i < current_command_spot; i++) {
			if (commands[i].commandName) {
				fprintf(ws_out_stream, "  %s\n", commands[i].commandName);
			}
		}
		fprintf(ws_out_stream, "\n");
	}
	ws_setConsoleColors(ws_getDefaultColors());
	return 0;
}

/* Internal history command */
const char* history_aliases[] = { "hist" };
/**
 * @internal
 * @brief History function help command
 */
int historyHelp(int argc, char** argv) {
	ws_help_entry_general_t entry = {
			"History",
			"Displays the terminal history. Limit of 32 previous commands.",
			NULL,
			0,
			history_aliases,
			1
	};
	ws_printGeneralHelp(&entry);
	return 0;
}

/**
 * @internal
 * @brief History function main command
 */
int historyMain(int argc, char** argv) {
	ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
	for (size_t i = 0; i < previous_commands_size; i++) {
		fprintf(ws_out_stream, "%s\n", previousCommands[i]);
	}
	ws_setConsoleColors(ws_getDefaultColors());
	return 0;
}

#ifdef THREADED_SUPPORT
ws_atomic_bool_t* exit_terminal = NULL;

/**
 * @internal
 * @brief Checks exit bool, makes sure it exists.
 */
void ws_internal_checkExitBool() {
	if (!exit_terminal) exit_terminal = ws_createAtomicBool(false);
}
#define CHECK_EXIT_BOOL_EXISTS ws_internal_checkExitBool()
#define GET_EXIT_BOOL ws_getAtomicBool(exit_terminal)
#define SET_EXIT_BOOL(b) ws_setAtomicBool(exit_terminal, b)

/**
 * @brief Stops the currently running terminal. Only supported in threaded applications.
 */
void ws_stopTerminal() { SET_EXIT_BOOL(true); }
#else
bool exit_terminal = false;
#define CHECK_EXIT_BOOL_EXISTS
#define GET_EXIT_BOOL exit_terminal
#define SET_EXIT_BOOL(b) exit_terminal = b
#endif

/* Internal exit command */
/**
 * @internal
 * @brief Exit function help command
 */
int exitHelp(int argc, char** argv) {
	const char* optional[] = {
			"--yes",
			"-y   -> Exits the terminal without the prompt."
	};
	ws_help_entry_specific_t entry = {
			"Exit",
			"Exits the terminal.",
			NULL,
			0,
			optional,
			2
	};
	ws_printSpecificHelp(&entry);
	return 0;
}

/**
 * @internal
 * @brief Exit function main command
 */
int exitMain(int argc, char** argv) {
	if (argc > 1) {
		if ((strcmp(argv[1], "-y") == 0 || strcmp(argv[1], "--yes") == 0)) {
			SET_EXIT_BOOL(true);
		} else {
			ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
			fprintf(ws_out_stream, "Unknown argument: %s\n", argv[1]);
			ws_setConsoleColors(ws_getDefaultColors());
		}
	} else {
		SET_EXIT_BOOL(ws_promptUser("Are you sure you want to exit?"));
	}
	printf("\n");
	return 0;
}

/**
 * @internal
 * @brief Registers all base commands.
 */
void ws_internal_registerBasicCommands() {
	// We static define the aliases for basic commands.
	// We dont use malloc because we dont want to deal with having to free anything
	// It also would be way messier for DISABLE_MALLOC if we did allocate things.
	// WallShell wants just the pointers, cleaning it up is the user's responsibility

	// a bare shell only has help, exit, clear, and history
	// might come up with some more overtime, such as echo, but it's not a big priority.
#ifndef NO_CLEAR_COMMAND
	ws_registerCommand((ws_command_t) { clearMain, clearHelp, "clear", clear_aliases, 2 });
#endif // NO_CLEAR_COMMAND

#ifndef NO_HELP_COMMAND
	ws_registerCommand((ws_command_t) { helpMain, helpHelp, "help", NULL, 0 });
#endif // NO_HELP_COMMAND

#ifndef NO_HISTORY_COMMAND
	ws_registerCommand((ws_command_t) { historyMain, historyHelp, "history", history_aliases, 1 });
#endif // NO_HISTORY_COMMAND

#ifndef NO_EXIT_COMMAND
	ws_registerCommand((ws_command_t) { exitMain, exitHelp, "exit", NULL, 0 });
#endif // NO_EXIT_COMMAND
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Virtual Sequences and Cursor Control
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/* Input */
typedef enum {
	NONE = 0,
	CURSOR,
	FUNCTION,
} input_type_t;

#ifndef CUSTOM_CURSOR_CONTROL
/**
 * @brief Move the cursor n times in the provided direction.
 * @param direction Direction to move the cursor, using ws_cursor_t.
 * @param n Amount of times to move in that direction.
 */
void ws_moveCursor_n(ws_cursor_t direction, size_t n) {
	switch (direction) {
		case WS_CURSOR_LEFT: {
				fprintf(ws_out_stream, "\033[%zuD", n);
				break;
			}
		case WS_CURSOR_RIGHT: {
				fprintf(ws_out_stream, "\033[%zuC", n);
				break;
			}
		case WS_CURSOR_UP: {
				fprintf(ws_out_stream, "\033[%zuA", n);
				break;
			}
		case WS_CURSOR_DOWN: {
				fprintf(ws_out_stream, "\033[%zuB", n);
				break;
			}
		default: break;
	}
}

/**
 * @brief Moves the cursor once in the provided direction.
 * @param direction Direction to move, using ws_cursor_t.
 */
void ws_moveCursor(ws_cursor_t direction) { ws_moveCursor_n(direction, 1); }
#endif // CUSTOM_CURSOR_CONTROL

typedef struct {
	input_type_t type;
	uint64_t result;
} input_result_t;

/**
 * @internal
 * @brief Processes a virtual terminal sequence
 *
 * @return input_result_t The type of input that the sequence was.
 */
input_result_t ws_internal_processVirtualSequence() {
	// The next character should be '[', and we can parse input until we know it should end with a certain character.
	// For simplicity's sake we're just going to preallocate a buffer for the input
	// If it doesn't end up being used it's not a big deal.
	input_result_t result = { NONE, 0 };
	int next = ws_get_char_blocking(ws_in_stream);
	if (next != '[' && next != 'O') {
		fprintf(ws_out_stream, "%c", next);
		return result;
	}

	char seq[10];
	int i = 0;

	// Read until we encounter a non-numeric character
	next = ws_get_char_blocking(ws_in_stream);
	while (next >= '0' && next <= '9' || next == ';') {
		seq[i++] = (char) next;
		next = ws_get_char_blocking(ws_in_stream);
	}
	seq[i] = '\0';

	// Handle the end character of the escape sequence
	switch (next) {
		case 'A': result.type = CURSOR;
			result.result = WS_CURSOR_UP;
			break;
		case 'B': result.type = CURSOR;
			result.result = WS_CURSOR_DOWN;
			break;
		case 'C': result.type = CURSOR;
			result.result = WS_CURSOR_RIGHT;
			break;
		case 'D': result.type = CURSOR;
			result.result = WS_CURSOR_LEFT;
			break;
			//case '~': printf("Function key, sequence: %s\n", seq);
			//	break;
			//case 'P':
			//case 'Q':
			//case 'R':
			//case 'S': printf("Special function key\n");
			//	break;
		default: break;
	}
	return result;
}

/**
 * @internal
 * @brief Process E0 keys. This is mostly for arrow keys in custom OS's and Windows.
 *
 * @return input_result_t
 */
input_result_t ws_internal_processEO() {
	// Up: 0x48 -> Down: 0x50 -> Right: 0x4d -> Left: 0x4b
	int next = ws_get_char_blocking(ws_in_stream);;
	input_result_t result = { NONE, 0 };
	switch (next) {
		case WS_CURSOR_UP:
		case WS_CURSOR_DOWN:
		case WS_CURSOR_LEFT:
		case WS_CURSOR_RIGHT: result.type = CURSOR;
			result.result = next;
			break;
		default: break;
	}
	return result;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Execute command & Main
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
 * @brief Execute a command with the provided buffer.
 * @param commandBuf Buffer containing the command to execute, including any flags, parameters, etc.
 * @return Can return WALLSHELL_OUT_OF_MEMORY if malloc returns NULL when DISABLE_MALLOC is not defined.
 */
ws_error_t ws_executeCommand(char* commandBuf) {
#ifdef DISABLE_MALLOC
	// Split the commandBuf into arguments based on spaces or other delimiters
	int argc = 0;
	char* argv[MAX_ARGS];
	char* current = strtok(commandBuf, " ");
	while (current != NULL) {
		if (argc >= MAX_ARGS) break;
		// allocates memory for the string and copies it
		argv[argc] = current;
		current = strtok(NULL, " ");
		argc++;
	}
#else
	// We treat this like system execution does with int argc & char** argv.
	// argv[0] is always the command name, argc always is at least 1 because of this
	int argc = 0;
	char** argv = NULL;
	char* current = strtok(commandBuf, " ");

	while (current != NULL) {
		char** newptr = (char**) realloc(argv, sizeof(char*) * (argc + 1));
		if (!newptr) {
			if (argv) {
				// free each string allocated by strdup
				for (int i = 0; i < argc; i++) free(argv[i]);
				free(argv);
			}
			return WALLSHELL_OUT_OF_MEMORY;
		} else {
			argv = newptr;
		}
		// allocates memory for the string and copies it
		char* str = malloc(strlen(current) + 1);
		strcpy(str, current);
		argv[argc] = str;
		current = strtok(NULL, " ");
		argc++;
	}
	if (argc == 0) {
		// Somehow got an empty command.
		if (argv) free(argv);
		return WALLSHELL_NO_ERROR;
	}
#endif // DISABLE_MALLOC
	// Call Command (if it exists)
	for (size_t i = 0; i < current_command_spot; i++) {
		if (commands[i].commandName && strcmp(commands[i].commandName, argv[0]) == 0) {
			int result = commands[i].mainCommand(argc, argv);
			if (result != 0) {
				// If the command function returns a non-zero value, it may indicate an error
				ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
				fprintf(ws_out_stream, "Command exited with code: %d\n", result);
			}
			goto cleanup;
		}

		// Check that commands alias
		for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
			if (commands[i].aliases[alias_idx] && strcmp(commands[i].aliases[alias_idx], argv[0]) == 0) {
				int result = commands[i].mainCommand(argc, argv);
				if (result != 0) {
					// If the command function returns a non-zero value, it may indicate an error
					ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "Command exited with code: %d\n", result);
				}
				goto cleanup;
			}
		}
	}
	ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
	fprintf(ws_out_stream, "Command not found: \"%s\"\n", argv[0]);
cleanup:
#ifndef DISABLE_MALLOC
	for (int i = 0; i < argc; i++) free(argv[i]);
	free(argv);
#endif // DISABLE_MALLOC
	ws_setConsoleColors(ws_getDefaultColors());
	return WALLSHELL_NO_ERROR;
}

// Default prefix
const char* prefix = "> ";
/**
 * @brief Set the prefix to the provided one.
 *
 * The prefix is what is displayed at the start of a command line.
 * It is possible to use this function to imitate a bash like `user@name:path$`, or any other combination.
 *
 * @param newPrefix
 */
void ws_setConsolePrefix(const char* newPrefix) { prefix = newPrefix; }

/**
 * @brief Cleans everything.
 *
 * Resets everything to it's default state, frees all allocations, etc.
 * Ideally you should call this before you exit, but the system garbage collector should clean it up.
 * Don't rely on the system gc for critical applications.
 */
void ws_cleanAll() {
	prefix = "> ";
	backspace_as_ascii_delete = false;
#ifdef THREADED_SUPPORT
	if (exit_terminal) ws_destroyAtomicBool(exit_terminal);
	exit_terminal = NULL;
#else
	exit_terminal = false;
#endif // THREADED_SUPPORT
	ws_internal_cleanStreams();
	ws_internal_cleanCommands();
	ws_internal_cleanColors();
#ifndef NO_LOGGING
	ws_internal_cleanLogger();
#endif // NO_LOGGING
	ws_resetConsoleState();
}

/**
 * @brief Main function for the terminal. Call after any configuration.
 * @return Can return WALLSHELL_OUT_OF_MEMORY if DISABLE_MALLOC is not defined, and malloc returns NULL.
 */
ws_error_t ws_terminalMain() {
	/* We're assuming that the user has printed everything they want prior to calling main. */
	/* We're also assuming the colors have been defined, even if they are blank. */
#ifndef NO_BASIC_COMMANDS
	ws_internal_registerBasicCommands();
#endif

	// Check for stream configurations
	if (!ws_err_stream) ws_setStream(WALLSHELL_ERROR, stderr);
	if (!ws_out_stream) ws_setStream(WALLSHELL_OUTPUT, stdout);
	if (!ws_in_stream) ws_setStream(WALLSHELL_INPUT, stdin);

#ifndef CUSTOM_WS_SETUP
	setConsoleMode();
#endif // CUSTOM_WS_SETUP

	// Make sure the colors are set properly if they are defaults
	ws_internal_updateColors();

	/* Ideally something should've caught this before calling main, but we still need to check. */
#ifndef DISABLE_MALLOC
	if (!commands) commands = malloc(sizeof(ws_command_t));
	if (!commands) return WALLSHELL_OUT_OF_MEMORY;
#endif
	bool newCommand = true;
	bool tabPressed = false; // allows for autocompletion

	size_t position_in_previous = 0;
	size_t current_position = 1;

	char commandBuf[MAX_COMMAND_BUF];
	char oldCommand[MAX_COMMAND_BUF];

	input_result_t input_result = { 0, 0 };

	while (!GET_EXIT_BOOL) {
		if (newCommand) {
			fprintf(ws_out_stream, "%s", prefix);
			newCommand = false;
			tabPressed = false;
			position_in_previous = 0;
			current_position = 1;
			memset(oldCommand, 0, MAX_COMMAND_BUF);
			memset(commandBuf, 0, MAX_COMMAND_BUF);
#ifdef PRINTING_NEEDS_FLUSH
			fflush(ws_out_stream);
#endif
		}

		// Check for the previous input results
		if (input_result.type != NONE) {
			if (input_result.type == CURSOR) {
				switch (input_result.result) {
					case WS_CURSOR_UP: {
							CLEAR_ROW;
							if (position_in_previous == 0) {
								memset(oldCommand, 0, MAX_COMMAND_BUF);
								memcpy(oldCommand, commandBuf, MAX_COMMAND_BUF);
							}
							memset(commandBuf, 0, MAX_COMMAND_BUF);
							memcpy(commandBuf, previousCommands[position_in_previous], strlen(previousCommands[position_in_previous]));
							fprintf(ws_out_stream, "\r%s%s", prefix, commandBuf);
							if (previous_commands_size > 0 && position_in_previous < previous_commands_size - 1) {
								position_in_previous++;
							}
							input_result.type = NONE;
							current_position = 1;
							continue;
						}
					case WS_CURSOR_DOWN: {
							CLEAR_ROW;
							if (previous_commands_size == 1 && position_in_previous == 1) position_in_previous--;
							if (position_in_previous > 0) {
								position_in_previous--;
								memset(commandBuf, 0, MAX_COMMAND_BUF);
								memcpy(commandBuf, previousCommands[position_in_previous], strlen(previousCommands[position_in_previous]));
							} else {
								memset(commandBuf, 0, MAX_COMMAND_BUF);
								memcpy(commandBuf, oldCommand, MAX_COMMAND_BUF);
							}
							fprintf(ws_out_stream, "\r%s%s", prefix, commandBuf);
							current_position = 1;
							input_result.type = NONE;
							continue;
						}
					case WS_CURSOR_RIGHT: {
							if (current_position == (strlen(commandBuf) + 1)) break;
							current_position++;
							ws_moveCursor(WS_CURSOR_RIGHT);
							input_result.type = NONE;
							continue;
						}
					case WS_CURSOR_LEFT: {
							if (current_position == 1) break;
							current_position--;
							ws_moveCursor(WS_CURSOR_LEFT);
							input_result.type = NONE;
							continue;
						}
					default: break;
				}
			}
#ifdef PRINTING_NEEDS_FLUSH
			fflush(ws_out_stream);
#endif
		}

		int current = ws_get_char(ws_in_stream);

		if (current == -2) {
			ws_sleep(10);
			continue;
		}

		if (backspace_as_ascii_delete && current == 0x7f)
			current = '\b';
		if (current == '\n' || current == '\r') {
			// If there's an empty command we just start a new line.
			fprintf(ws_out_stream, "\n");
			if (strlen(commandBuf) == 0) {
				newCommand = true;
				continue;
			}

			// Move everything right in the previous buf
			if (previous_commands_size > 0) {
				if (strcmp(previousCommands[0], commandBuf) != 0) {
					for (size_t i = previous_commands_size; i > 0; i--) {
						memcpy(previousCommands[i], previousCommands[i - 1], strlen(previousCommands[i - 1]));
						memset(previousCommands[i - 1], 0, MAX_COMMAND_BUF);
					}

					if (previous_commands_size < PREVIOUS_BUF_SIZE) {
						previous_commands_size++;
					}
				}
				memcpy(previousCommands[0], commandBuf, strlen(commandBuf));
			} else {
				previous_commands_size++;
				memcpy(previousCommands[0], commandBuf, strlen(commandBuf));
			}
			ws_executeCommand(commandBuf);
			commandBuf[0] = '\0';
			newCommand = true;
		} else if (current == '\b') {
			if (strlen(commandBuf) > 0) {
				if (current_position <= 1) continue;
				// Remove the current position & shift to the left
				size_t len = strlen(commandBuf);
				for (size_t i = current_position - 2; i < len; i++) {
					commandBuf[i] = commandBuf[i + 1];
				}
				// Ensure it's null terminated.
				// In theory, it should already be, but I'd rather do this unnecessary step than have an overflow or messed up buffer.
				commandBuf[len - 1] = '\0';

				current_position--;
				if (current_position != (strlen(commandBuf) + 1)) {
					CLEAR_ROW;
					fprintf(ws_out_stream, "%s%s", prefix, commandBuf);
					ws_moveCursor(WS_CURSOR_LEFT);
					for (size_t i = strlen(commandBuf); i > current_position; i--) {
						ws_moveCursor(WS_CURSOR_LEFT);
					}
				} else {
					// only clear the last char. much quicker than rewriting the line
					ws_moveCursor(WS_CURSOR_LEFT);
					fprintf(ws_out_stream, " ");
					ws_moveCursor(WS_CURSOR_LEFT);
				}
			}
		} else if (current == '\t') {
			// see if we can autocomplete a command.
			const char* list[50]; // List of current possible commands
			int list_size = 0;
			for (int i = 0; i < command_size; i++) {
				ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT });
				if (commands[i].commandName && ws_internal_startsWith(commands[i].commandName, commandBuf)) {
					list[list_size] = commands[i].commandName;
					list_size++;
				}

				// Check aliases for a match
				for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
					if (commands[i].aliases[alias_idx] && ws_internal_startsWith(commands[i].aliases[alias_idx], commandBuf)) {
						// If it's an alias of a command that's already in the list, we dont want it.
						bool already_in_list = false;
						for (int j = 0; j < list_size; j++) {
							if (strcmp(list[j], commands[i].commandName) == 0) {
								already_in_list = true;
								break;
							}
						}
						if (!already_in_list) {
							list[list_size] = commands[i].aliases[alias_idx];
							list_size++;
						}
					}
				}
				ws_setConsoleColors(ws_getDefaultColors());
			}

			if (list_size == 1) {
				// Print the rest of the command
				size_t len = strlen(commandBuf);
				const char* currentCommand = list[0];
				for (size_t i = len; i < strlen(currentCommand); i++) {
					fprintf(ws_out_stream, "%c", currentCommand[i]);
					ws_internal_strcat_c(commandBuf, currentCommand[i], MAX_COMMAND_BUF);
				}
				tabPressed = false;
			} else if (tabPressed) {
				if (list_size == 0) {
					ws_setConsoleColors((ws_color_t) { WS_FG_BRIGHT_RED, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "\nNo command starting with: %s\n", commandBuf);
					// Clear the buffer
					memset(commandBuf, 0, MAX_COMMAND_BUF * sizeof(char));
					commandBuf[0] = '\0';
					newCommand = true;
				} else if (list_size > 1) {
					// Print out all commands
					ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
					fprintf(ws_out_stream, "\n");
					for (int i = 0; i < list_size; i++) {
						fprintf(ws_out_stream, "%s\n", list[i]);
					}
					ws_setConsoleColors(ws_getDefaultColors());
					// Reprint the command line
					fprintf(ws_out_stream, "\r%s%s", prefix, commandBuf);
				}
				tabPressed = false;
			} else {
				tabPressed = true;
			}
			ws_setConsoleColors(ws_getDefaultColors());
		} else if (current == EOF) {
			// Temporarily for development’s sake, this is how you exit the console.
			// ctrl+d on unix, ctrl+z on windows
			break;
		} else if (current == '\033') {
			input_result = ws_internal_processVirtualSequence();
		} else if (current == 0xE0) {
			// Microsoft sometimes wants to work with virtual inputs but usually doesn't.
			// At the very least this makes porting it to an os very easy.
			// All the OS has to do is give this program raw input in the form of scancodes for special keys.
			input_result = ws_internal_processEO();
		} else {

			ws_internal_insert_c(commandBuf, MAX_COMMAND_BUF, (char) current, current_position);
			if (current_position != strlen(commandBuf)) {
				CLEAR_ROW;
				fprintf(ws_out_stream, "%s%s", prefix, commandBuf);
				for (size_t i = strlen(commandBuf); i > current_position; i--) {
					ws_moveCursor(WS_CURSOR_LEFT);
				}
			} else {
				fprintf(ws_out_stream, "%c", current);
			}
			current_position++;
			//printf("current command: %s -> size: %llu -> pos: %zu\n", commandBuf, strlen(commandBuf), current_position);
		}
#ifdef PRINTING_NEEDS_FLUSH
		fflush(ws_out_stream);
#endif
	}
	return WALLSHELL_NO_ERROR;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// General Utility functions
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
 * @brief Operator overloading isn't available in C. Compare two commands using this.
 *
 * @param c1 Command to compare.
 * @param c2 Command to compare.
 * @return true If they are the same.
 * @return false If they are different.
 */
bool ws_compareCommands(const ws_command_t c1, const ws_command_t c2) {
	if (c1.aliases != c2.aliases) return false;
	if (c1.aliases_count != c2.aliases_count) return false;
	if (strcmp(c1.commandName, c2.commandName) != 0) return false;
	if (c1.helpCommand != c2.helpCommand) return false;
	if (c1.mainCommand != c2.mainCommand) return false;
	return true;
}

/**
 * @brief Sets the console locale. This is only required on Windows systems, as terminals default to ASCII.
 *
 * The built in SET_TERMINAL_LOCALE sets the Windows terminal to UTF8.
 * Can be potentially be used on unix systems to configure locale, although this isn't done by default.
 * If the system you are implementing requires locale configuration, redefine SET_TERMINAL_LOCALE to the needed configuration.
 */
void ws_setConsoleLocale() { SET_TERMINAL_LOCALE; }

/**
 * @anchor general help
 * @brief Prints the general help entry.
 * @param entry Entry to be displayed. If you don't want sections displayed, mark them as NULL in the struct.
 */
void ws_printGeneralHelp(ws_help_entry_general_t* entry) {
	// Command Name
	ws_setConsoleColors((ws_color_t) { WS_FG_RED, WS_BG_DEFAULT });
	if (entry->commandName)
		fprintf(ws_out_stream, "\n%s\n", entry->commandName);

	// Description
	ws_setConsoleColors((ws_color_t) { WS_FG_CYAN, WS_BG_DEFAULT });
	if (entry->description)
		fprintf(ws_out_stream, "%s\n", entry->description);

	// Commands
	if (entry->commands_count > 0) {
		ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "\nCommands:\n");

		ws_setConsoleColors((ws_color_t) { WS_FG_GREEN, WS_BG_DEFAULT });
		for (int i = 0; i < entry->commands_count; i++) {
			if (entry->commands[i])
				fprintf(ws_out_stream, "  %s\n", entry->commands[i]);
		}
	}

	// Aliases
	if (entry->aliases_count > 0) {
		ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "\nAliases:\n");

		ws_setConsoleColors((ws_color_t) { WS_FG_GREEN, WS_BG_DEFAULT });
		for (int i = 0; i < entry->aliases_count; i++) {
			if (entry->aliases[i])
				fprintf(ws_out_stream, "  %s\n", entry->aliases[i]);
		}
	}
	ws_setConsoleColors((ws_color_t) { WS_FG_DEFAULT, WS_BG_DEFAULT });
	fprintf(ws_out_stream, "\n");
}

/**
 * @brief Prints the specific help entry.
 * @param entry Entry to be displayed. If you don't want sections displayed, mark them as NULL in the struct.
 * @anchor specific_help
 */
void ws_printSpecificHelp(ws_help_entry_specific_t* entry) {
	// Command Name
	ws_setConsoleColors((ws_color_t) { WS_FG_RED, WS_BG_DEFAULT });
	if (entry->commandName)
		fprintf(ws_out_stream, "\n%s\n", entry->commandName);

	// Description
	ws_setConsoleColors((ws_color_t) { WS_FG_CYAN, WS_BG_DEFAULT });
	if (entry->description)
		fprintf(ws_out_stream, "%s\n", entry->description);

	// Commands
	if (entry->required_count > 0) {
		ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "Required:\n");

		ws_setConsoleColors((ws_color_t) { WS_FG_GREEN, WS_BG_DEFAULT });
		for (int i = 0; i < entry->required_count; i++) {
			if (entry->required[i])
				fprintf(ws_out_stream, "  %s\n", entry->required[i]);
		}
	}

	// Aliases
	if (entry->optional_count > 0) {
		ws_setConsoleColors((ws_color_t) { WS_FG_YELLOW, WS_BG_DEFAULT });
		fprintf(ws_out_stream, "\nOptional:\n");

		ws_setConsoleColors((ws_color_t) { WS_FG_GREEN, WS_BG_DEFAULT });
		for (int i = 0; i < entry->optional_count; i++) {
			if (entry->optional[i])
				fprintf(ws_out_stream, "  %s\n", entry->optional[i]);
		}
	}
	ws_setConsoleColors((ws_color_t) { WS_FG_DEFAULT, WS_BG_DEFAULT });
	fprintf(ws_out_stream, "\n");
}

/**
 * @brief Prompts the user yes/no using the given prompt.
 * @param format Printf style formatting string.
 * @param ... Printf style arguments.
 * @return True if the user reply's yes, false otherwise. Will return false if the user enters anything other than something starting with 'Y' or 'y'.
 */
bool ws_promptUser(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	vfprintf(ws_out_stream, format, arg);
	va_end(arg);

	fprintf(ws_out_stream, " [Y/n] ");
	int first_input;
	do {
		first_input = ws_get_char(ws_in_stream);
		ws_sleep(10);
	} while (first_input == -2);
	fprintf(ws_out_stream, "%c", first_input);
	int input;
	do {
		do {
			input = ws_get_char(ws_in_stream);
		} while (input == -2);
		fprintf(ws_out_stream, "%c", input);
	} while (input != '\n');
	if (first_input == 'Y' || first_input == 'y') return true;
	return false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// The bottom of this file is to document things for Doxygen that aren't necessarily able to be
// documeneted elsewhere, or are otherwise messy to do so. This includes things like structs, 
// typedefs, unions, etc. Anything that isn't documented in place should be done so here. 
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
 * @struct ws_command_t wall_shell.h
 * @brief Command Data Structure
 *
 * This structure holds all data related to the command. There are two required entires, `mainCommand` and `commandName`.
 * The rest of the structures are optional, and if they are unused should be set to `NULL`, `nullptr`, or `0` respectively.
 *
 * @var ws_command_t::mainCommand
 * @brief Main function of the command. This is called when your command is written in terminal.
 *
 * Should be a pointer to a C style main function.
 * This means it needs to be in the form `int func(int argc, char** argv);`
 * If you are using this in C++, you will have to mark it `extern "c"`, and may have to do some trickery if it's part of a class.
 * `argc` and `argv` work identically to those in the C-standard.
 * The return code is displayed to terminal if it isn't 0.
 *
 * @var ws_command_t::helpCommand
 * @brief Help function of the command. This is not required. This is called when `help <command>` is called.
 *
 * Should be a pointer to a C style main function.
 * This means it needs to be in the form `int func(int argc, char** argv);`
 * If you are using this in C++, you will have to mark it `extern "C"`, and may have to do some trickery if it's part of a class.
 * `argc` and `argv` work identically to those in the C-standard.
 * The return code is displayed to terminal if it isn't 0.
 *
 * @var ws_command_t::commandName
 * @brief Name of the command. This is what the user has to type into the terminal to run your command.
 *
 * @var ws_command_t::aliases
 * @brief Any aliases you want your command to have. When typed, these call your command as if they are the normal Command Name.
 * @warning Make sure to not declare this in context.
 * If declared in context, and that scope is exited, WallShell will be trying to read non-existent entries, likely causing a segfault.
 * Create this array using malloc (or calloc) if available, or declare it in a global scope.
 *
 * @var ws_command_t::aliases_count
 * @brief Amount of aliases in your alias array.
 *
 * This should act like strlen, it's the count, not the indexes.
 * If this number is inaccurate, WallShell will either not read all you entries (if count is too small) or create a buffer overflow (and likely a segfault).
 */

/**
 * @struct ws_help_entry_general_t wall_shell.h
 * @brief General help structure.
 *
 * Makes printing help menu entries consistent across different functions.
 * Not all fields have to be filled in. You can leave fields as `NULL` to not print them.
 * This is meant for general command help entries, mostly meaning first level commands (like clear, exit, etc.)
 *
 * This is often used to define "Categories".
 * Say you have several time functions.
 * Each one could set it's help function to the same thing, which in turn prints this structure listing out all `time` commands.
 * You can also use it to show sub-commands in a similar way.
 * Using git as an example, you could have `help git` print this structure,
 * while listing things like `git branch` under the commands part of this struct.
 *
 * Meant to be used with @ref ws_printGeneralHelp().
 *
 * @var ws_help_entry_general_t::commandName
 * @brief Name of the command.
 *
 * @var ws_help_entry_general_t::description
 * @brief Description of the command.
 *
 * @var ws_help_entry_general_t::commands
 * @brief Commands that are a subset of this command/topic.
 *
 * @var ws_help_entry_general_t::commands_count
 * @brief Amount of commands in the `commands` part of this structure. It's the count, not the indexes.
 *
 * @var ws_help_entry_general_t::aliases
 * @brief Any aliases that this command has.
 * @note It's generally a good idea to use the same `aliases` array that you used when creating the @ref ws_command_t related to this command.
 *
 * @var ws_help_entry_general_t::aliases_count
 * @brief Amount of commands in the `aliases` part of this structure. It's the count, not the indexes.
 */

/**
 * @struct ws_help_entry_specific_t wall_shell.h
 * Specific help structure. Makes printing help menu entries consistent across different functions.
 * Not all fields have to be filled in. You can leave fields as NULL to not print them.
 *
 * This is meant for things like subcommands, flags, etc.
 *
 * Meant to be used with @ref ws_printSpecificHelp().
 *
 * @var ws_help_entry_specific_t::commandName
 * @brief Name of the command.
 *
 * @var ws_help_entry_specific_t::description
 * @brief Description of the command.
 *
 * @var ws_help_entry_specific_t::required
 * @brief Flags/subcommands that are a required when running this command.
 *
 * @var ws_help_entry_specific_t::required_count
 * @brief Amount of flags/subcommands in the `required` part of this structure. It's the count, not the indexes.
 *
 * @var ws_help_entry_specific_t::optional
 * @brief Flags/subcommands that are optional when running this command.
 *
 * @var ws_help_entry_specific_t::optional_count
 * @brief Amount of flags/subcommands in the `optional` part of this structure. It's the count, not the indexes.
 */

/**
 * @struct ws_color_t wall_shell.h
 * @brief Color structure used internally by WallShell
 *
 * All internal color settings use this, along with allowing the user the ability to set both foreground and background at the same time.
 *
 * @var ws_color_t::foreground
 * @brief Foreground color of the structure.
 *
 * @var ws_color_t::background
 * @brief Background color of the structure.
 */

/**
 * @struct ws_atomic_bool_t wall_shell.h
 * @brief Atomic bool structure provided by WallShell.
 *
 * This is simply a wrapper around your system's mutex type. How it's created, destroyed, and accessed is very important.
 *
 * @warning Even though they are exposed, you should never access member varibles directly.
 * Make sure you use @ref ws_getAtomicBool() and @ref ws_setAtomicBool().
 * Create & destory it using @ref ws_createAtomicBool() and @ref ws_destroyAtomicBool() respectively.
 *
 * @var ws_atomic_bool_t::b
 * @brief  Bool currently stored by the atomic_bool. This should never be accessed directly.
 *
 * @var ws_atomic_bool_t::mut
 * @brief Pointer to the mutex the bool uses. This should never be accessed directly.
 */