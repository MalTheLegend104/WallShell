/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
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
 * @brief Internal function to concat a single char to the end of a string.
 * If the string is already at max length (including room for '\0') then the string is returned unchanged.
 *
 * @param string String to add the character to.
 * @param c Character to add to the string.
 * @param size Size of the entire buffer.
 * @return char* Same as the string passed through.
 */
char* wallshell_internal_strcat_c(char* string, char c, size_t size) {
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
 * @brief Internal function to insert a single char into a string.
 * If the string is already at max length (including room for '\0') then the string is returned unchanged.
 *
 * @param string String to add the character to.
 * @param buf_size Size of the entire buffer.
 * @param c Character to add to the string.
 * @param position Position that the character is inserted into. It should be index + 1
 * @return char* Same as the string passed through.
 */
char* wallshell_internal_insert_c(char* string, size_t buf_size, char c, size_t position) {
	size_t current_length = strlen(string);
	if (current_length + 1 >= buf_size) return string;
	
	for (size_t i = current_length; i > position - 1; i--) {
		string[i] = string[i - 1];
	}
	string[position - 1] = c;
	
	return string;
}

bool wallshell_internal_startsWith(const char* str, const char* prefix) {
	while (*prefix) {
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
FILE* wallshell_out_stream = NULL;
FILE* wallshell_err_stream = NULL;
FILE* wallshell_in_stream = NULL;

void setStream(wallshell_stream type, FILE* stream) {
	switch (type) {
		case WALLSHELL_INPUT: wallshell_in_stream = stream;
			break;
		case WALLSHELL_OUTPUT: wallshell_out_stream = stream;
			break;
		case WALLSHELL_ERROR: wallshell_err_stream = stream;
			break;
		default: break;
	}
}

void initializeDefaultStreams() {
	setStream(WALLSHELL_INPUT, stdin);
	setStream(WALLSHELL_ERROR, stderr);
	setStream(WALLSHELL_OUTPUT, stdout);
}

#ifndef CLEAR_ROW
	#define CLEAR_ROW fprintf(wallshell_out_stream, "\033[M");
#endif // CLEAR_ROW

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Custom Console Setup
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// For some systems (mostly POSIX), backspace gets sent as ascii delete rather than \b
bool backspace_as_ascii_delete = false;
#ifndef CUSTOM_CONSOLE_SETUP
	#ifndef _WIN32
		#include <termios.h>
		#include <unistd.h>
struct termios old_settings, new_settings;
	#endif // _WIN32
wallshell_error_t setConsoleMode() {
#ifdef _WIN32
	// Set output mode to handle virtual terminal sequences
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return WALLSHELL_CONSOLE_SETUP_ERROR;
	
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	if (hIn == INVALID_HANDLE_VALUE) return WALLSHELL_CONSOLE_SETUP_ERROR;
	
	DWORD dwOriginalOutMode = 0;
	DWORD dwOriginalInMode = 0;
	if (!GetConsoleMode(hOut, &dwOriginalOutMode)) return WALLSHELL_CONSOLE_SETUP_ERROR;
	if (!GetConsoleMode(hIn, &dwOriginalInMode)) return WALLSHELL_CONSOLE_SETUP_ERROR;
	
	DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;
	
	DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	if (!SetConsoleMode(hOut, dwOutMode)) {
		// we failed to set both modes, try to step down mode gracefully.
		dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
		if (!SetConsoleMode(hOut, dwOutMode)) {
			// Failed to set any VT mode, can't do anything here.
			return WALLSHELL_CONSOLE_SETUP_ERROR;
		}
	}
	
	DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
	if (!SetConsoleMode(hIn, dwInMode)) {
		// Failed to set VT input mode, can't do anything here.
		return WALLSHELL_CONSOLE_SETUP_ERROR;
	}
	
	// Disable a few features that make a command handler hard to make
	DWORD inMode = 0;
	if (!GetConsoleMode(hIn, &inMode)) { return WALLSHELL_CONSOLE_SETUP_ERROR; }
	inMode &= ~ENABLE_LINE_INPUT;
	inMode &= ~ENABLE_PROCESSED_INPUT;
	inMode &= ~ENABLE_ECHO_INPUT;
	if (!SetConsoleMode(hIn, inMode)) { return WALLSHELL_CONSOLE_SETUP_ERROR; }
	fprintf(wallshell_out_stream, "\033=\033[?1h");
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

void resetConsoleState() {
#ifndef _WIN32
	// Restore original terminal settings
	tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
#endif // _WIN32
}
	
	#ifdef _WIN32
		#include <conio.h>
		#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
		#define wallshell_get_char(stream) _getch()
	#else
		#define wallshell_get_char(stream) getchar()
		#define SET_TERMINAL_LOCALE
	#endif // _WIN32
#endif // CUSTOM_CONSOLE_SETUP

// To aid portability, we allow the user to set backspace_as_ascii_delete
/**
 * @brief Some consoles send backspace as ASCII delete (0x7f) instead of '\b'.
 * If your system does this, set this to true. This only needs to be done if CUSTOM_CONSOLE_SETUP is defined.
 * POSIX and Windows based systems are automatically configured.
 * @param b Bool to set backspace_as_ascii_delete to.
 */
void setAsciiDeleteAsBackspace(bool b) { backspace_as_ascii_delete = b; }

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Default console color output.
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifndef CUSTOM_CONSOLE_COLORS
	#ifdef _WIN32
/* I'm trying to include the least amount of windows headers as possible */
		#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		#define SET_TERMINAL_LOCALE
	#endif
/* Modern windows is supposed to support these escape codes, older windows versions use SetConsoleTextAttribute */
/* https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences */
	#define RESET_CONSOLE                   fprintf(wallshell_out_stream, "\033[0m")

void changeConsoleColor(console_fg_color_t fg, console_bg_color_t bg) {
	if (fg == CONSOLE_FG_DEFAULT || bg == CONSOLE_BG_DEFAULT) {
		RESET_CONSOLE;
	}
	
	if (fg == CONSOLE_FG_DEFAULT && bg != CONSOLE_BG_DEFAULT) {
		fprintf(wallshell_out_stream, "\033[%dm", bg);
	} else if (fg != CONSOLE_FG_DEFAULT && bg == CONSOLE_BG_DEFAULT) {
		fprintf(wallshell_out_stream, "\033[%dm", fg);
	} else {
		fprintf(wallshell_out_stream, "\033[%d;%dm", fg, bg);
	}
}
	
	#define SET_CONSOLE_COLORS(a, b) changeConsoleColor(a, b);

#endif // CUSTOM_CONSOLE_COLORS

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Buffer declarations. Also checks for malloc usage to configure the buffers.
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifdef DISABLE_MALLOC
command_t commands[COMMAND_LIMIT];
size_t command_size = COMMAND_LIMIT;
#else
command_t* commands;
size_t command_size = 0;
#endif

size_t current_command_spot = 0;
size_t amount_of_commands;

char previousCommands[PREVIOUS_BUF_SIZE][MAX_COMMAND_BUF];
size_t previous_commands_size;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Console Colors
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
console_color_t default_colors = {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT};
console_color_t current_colors = {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT};

wallshell_error_t updateColors() {
	if (!wallshell_out_stream) return WALLSHELL_OUT_STREAM_NOT_SET;
	if (current_colors.foreground == CONSOLE_FG_DEFAULT) {
		current_colors.foreground = default_colors.foreground;
	}
	if (current_colors.background == CONSOLE_BG_DEFAULT) {
		current_colors.background = default_colors.background;
	}
	SET_CONSOLE_COLORS(current_colors.foreground, current_colors.background);
	return WALLSHELL_NO_ERROR;
}

void setConsoleForegroundDefault(console_fg_color_t c) {
	default_colors.foreground = c;
}
void setConsoleBackgroundDefault(console_bg_color_t c) {
	default_colors.background = c;
}

void setConsoleDefaults(console_color_t c) {
	setConsoleForegroundDefault(c.foreground);
	setConsoleBackgroundDefault(c.background);
}

console_color_t getCurrentColors() { return current_colors; }
console_color_t getDefaultColors() { return default_colors; }

wallshell_error_t setConsoleColors(console_color_t colors) {
	current_colors.foreground = colors.foreground;
	current_colors.background = colors.background;
	return updateColors();
}
wallshell_error_t setForegroundColor(console_fg_color_t color) {
	current_colors.foreground = color;
	return updateColors();
}
wallshell_error_t setBackgroundColor(console_bg_color_t color) {
	current_colors.background = color;
	return updateColors();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Register Command & Internal Commands
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

/**
 * @brief Register the command to the command handler.
 * @param c Command to be registered.
 */
wallshell_error_t registerCommand(const command_t c) {
#ifdef DISABLE_MALLOC
	if (current_command_spot != COMMAND_LIMIT) {
		commands[current_command_spot] = c;
		current_command_spot++;
	} else {
		return WALLSHELL_COMMAND_LIMIT_REACHED;
	}
#else
	if (!commands) {
		commands = malloc(sizeof(command_t));
		command_size = 1;
	} else if (current_command_spot >= command_size) {
		bool was_one = false;
		if (command_size == 1) {
			was_one = true;
			command_size++;
		}
		// realloc invalidates the old pointer on call, but leaves it alone if it cant find the memory.
		// If this function returns with an out of memory error, the shell is still usable.
		command_t* new_ptr = realloc(commands, (size_t) ((double) command_size * sizeof(command_t) * 1.5));
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

void deregisterCommand(const command_t c) {

}

int test(int argc, char** argv) {
	fprintf(wallshell_out_stream, "ooga booga: %s\n", argv[0]);
	for (int i = 0; i < argc; i++) {
		fprintf(wallshell_out_stream, "\t%s\n", argv[i]);
	}
	return -1;
}

const char* clear_aliases[] = {"clr", "cls"};
int clearHelp(int argc, char** argv) {
	help_entry_general_t entry = {
			"Clear",
			"Clears the screen",
			NULL,
			0,
			clear_aliases,
			2
	};
	printGeneralHelp(&entry);
	return 0;
}
int clearMain(int argc, char** argv) {
#ifdef _WIN32
	// Windows being windows, some escape characters don't work normally in like 2/3 of the terminals
	// This is especially evident in things spawned by AllocConsole()
	system("cls");
#else
	// Unix is much nicer
	fprintf(wallshell_out_stream, "\033c");
#endif
	// Sometimes clearing the screen results in the colors getting reset.
	updateColors();
	return 0;
}

void helpSearch(char* str) {
	setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
	fprintf(wallshell_out_stream, "List of commands starting with \"%s\": (A) indicates an alias.\n", str);
	setConsoleColors(getDefaultColors());
	for (int i = 0; i < current_command_spot; i++) {
		setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_GREEN, CONSOLE_BG_DEFAULT});
		if (commands[i].commandName && wallshell_internal_startsWith(commands[i].commandName, str)) {
			fprintf(wallshell_out_stream, "\t%s\n", commands[i].commandName);
		}
		
		// Check aliases for a match
		for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
			if (commands[i].aliases[alias_idx] && wallshell_internal_startsWith(commands[i].aliases[alias_idx], str)) {
				fprintf(wallshell_out_stream, "\t%s (A)\n", commands[i].aliases[alias_idx]);
			}
		}
		setConsoleColors(getDefaultColors());
	}
}

int helpHelp(int argc, char** argv) {
	const char* optional[] = {
			"-s <string> -> Lists all commands and aliases that start with <string>."
	};
	help_entry_specific_t entry = {
			"Help",
			"The help menu.",
			NULL,
			0,
			optional,
			1
	};
	printSpecificHelp(&entry);
	return 0;
}
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
					setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "Search flag must be followed by an argument.\n");
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
					setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "Command \"%s\" does not have a help function.\n", argv[0]);
					setConsoleColors(getDefaultColors());
					return 0;
				}
				
				// Execute the help command associated with the matched command
				int result = commands[i].helpCommand(argc, argv);
				if (result != 0) {
					// If the command function returns a non-zero value, it may indicate an error
					setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "Command exited with code: %d\n", result);
					setConsoleColors(getDefaultColors());
				}
				return 0;
			}
			
			// Check aliases for a match
			for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
				if (commands[i].aliases[alias_idx] && strcmp(commands[i].aliases[alias_idx], argv[0]) == 0) {
					// No help function for command.
					if (!commands[i].helpCommand) {
						setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
						fprintf(wallshell_out_stream, "Command \"%s\" does not have a help function.\n", argv[0]);
						setConsoleColors(getDefaultColors());
						return 0;
					}
					
					// Execute the help command associated with the matched alias
					int result = commands[i].helpCommand(argc, argv);
					if (result != 0) {
						// If the command function returns a non-zero value, it may indicate an error
						setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
						fprintf(wallshell_out_stream, "Command exited with code: %d\n", result);
						setConsoleColors(getDefaultColors());
					}
					return 0;
				}
			}
		}
		// If the command is not found in the registered commands or their aliases
		setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "Help command not found for: %s\n", argv[0]);
	} else {
		fprintf(wallshell_out_stream, "\n");
		setConsoleColors((console_color_t) {CONSOLE_FG_CYAN, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "To get more info about a command, run `help <command_name>`\n");
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "All commands:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_GREEN, CONSOLE_BG_DEFAULT});
		// List all available commands
		for (int i = 0; i < current_command_spot; i++) {
			if (commands[i].commandName) {
				fprintf(wallshell_out_stream, "  %s\n", commands[i].commandName);
			}
		}
		fprintf(wallshell_out_stream, "\n");
	}
	setConsoleColors(getDefaultColors());
	return 0;
}

const char* history_aliases[] = {"hist"};
int historyHelp(int argc, char** argv) {
	help_entry_general_t entry = {
			"History",
			"Displays the terminal history. Limit of 32 previous commands.",
			NULL,
			0,
			history_aliases,
			1
	};
	printGeneralHelp(&entry);
	return 0;
}
int historyMain(int argc, char** argv) {
	setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
	for (size_t i = 0; i < previous_commands_size; i++) {
		printf("%s\n", previousCommands[i]);
	}
	setConsoleColors(getDefaultColors());
	return 0;
}

bool exit_terminal = false;
int exitMain(int argc, char** argv) {
	if (argc > 1) {
		if ((strcmp(argv[1], "-y") == 0 || strcmp(argv[1], "--yes") == 0)) {
			exit_terminal = true;
		} else {
			setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
			fprintf(wallshell_out_stream, "Unknown argument: %s\n", argv[1]);
			setConsoleColors(getDefaultColors());
		}
	} else {
		exit_terminal = promptUser("Are you sure you want to exit?");
	}
	printf("\n");
	return 0;
}
int exitHelp() {
	const char* optional[] = {
			"--yes",
			"-y   -> Exits the terminal without the prompt."
	};
	help_entry_specific_t entry = {
			"Exit",
			"Exits the terminal.",
			NULL,
			0,
			optional,
			2
	};
	printSpecificHelp(&entry);
	return 0;
}

// We static define the aliases for basic commands.
// We dont use malloc because we dont want to deal with having to free anything
// WallShell wants just the pointers, cleaning it up is the user's responsibility
void registerBasicCommands() {
	// a bare shell only has help, exit, clear, and history
	// might come up with some more overtime, such as echo, but it's not a big priority.
	registerCommand((command_t) {test, NULL, "test", NULL, 0});
#ifndef NO_CLEAR_COMMAND
	registerCommand((command_t) {clearMain, clearHelp, "clear", clear_aliases, 2});
#endif // NO_CLEAR_COMMAND
#ifndef NO_HELP_COMMAND
	registerCommand((command_t) {helpMain, helpHelp, "help", NULL, 0});
#endif // NO_HELP_COMMAND
#ifndef NO_HISTORY_COMMAND
	registerCommand((command_t) {historyMain, historyHelp, "history", history_aliases, 1});
#endif // NO_HISTORY_COMMAND

#ifndef NO_EXIT_COMMAND
	registerCommand((command_t) {exitMain, exitHelp, "exit", NULL, 0});
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

void wallshell_move_cursor(console_cursor_t direction) {
	switch (direction) {
		case CONSOLE_CURSOR_LEFT: {
			fprintf(wallshell_out_stream, "\033[D");
			break;
		}
		case CONSOLE_CURSOR_RIGHT: {
			fprintf(wallshell_out_stream, "\033[C");
			break;
		}
		case CONSOLE_CURSOR_UP: {
			fprintf(wallshell_out_stream, "\033[A");
			break;
		}
		case CONSOLE_CURSOR_DOWN: {
			fprintf(wallshell_out_stream, "\033[B");
			break;
		}
		default: break;
	}
}

typedef struct {
	input_type_t type;
	uint64_t result;
} input_result_t;

input_result_t processVirtualSequence() {
	// The next character should be '[', and we can parse input until we know it should end with a certain character.
	// For simplicity's sake we're just going to preallocate a buffer for the input
	// If it doesn't end up being used it's not a big deal.
	input_result_t result = {NONE, 0};
	int next = wallshell_get_char(wallshell_in_stream);
	if (next != '[' && next != 'O') {
		fprintf(wallshell_out_stream, "%c", next);
		return result;
	}
	
	char seq[10];
	int i = 0;
	
	// Read until we encounter a non-numeric character
	next = wallshell_get_char(wallshell_in_stream);
	while (next >= '0' && next <= '9' || next == ';') {
		seq[i++] = (char) next;
		next = wallshell_get_char(wallshell_in_stream);
	}
	seq[i] = '\0';
	
	// Handle the end character of the escape sequence
	switch (next) {
		case 'A': result.type = CURSOR;
			result.result = CONSOLE_CURSOR_UP;
			break;
		case 'B': result.type = CURSOR;
			result.result = CONSOLE_CURSOR_DOWN;
			break;
		case 'C': result.type = CURSOR;
			result.result = CONSOLE_CURSOR_RIGHT;
			break;
		case 'D': result.type = CURSOR;
			result.result = CONSOLE_CURSOR_LEFT;
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

input_result_t processEO() {
	// Up: 0x48 -> Down: 0x50 -> Right: 0x4d -> Left: 0x4b
	int next = wallshell_get_char(wallshell_in_stream);
	input_result_t result = {NONE, 0};
	switch (next) {
		case CONSOLE_CURSOR_UP:
		case CONSOLE_CURSOR_DOWN:
		case CONSOLE_CURSOR_LEFT:
		case CONSOLE_CURSOR_RIGHT: result.type = CURSOR;
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
wallshell_error_t executeCommand(char* commandBuf) {
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
				setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
				fprintf(wallshell_out_stream, "Command exited with code: %d\n", result);
			}
			goto cleanup;
		}
		
		// Check that commands alias
		for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
			if (commands[i].aliases[alias_idx] && strcmp(commands[i].aliases[alias_idx], argv[0]) == 0) {
				int result = commands[i].mainCommand(argc, argv);
				if (result != 0) {
					// If the command function returns a non-zero value, it may indicate an error
					setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "Command exited with code: %d\n", result);
				}
				goto cleanup;
			}
		}
	}
	setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
	fprintf(wallshell_out_stream, "Command not found: \"%s\"\n", argv[0]);
cleanup:
#ifndef DISABLE_MALLOC
	for (int i = 0; i < argc; i++) free(argv[i]);
	free(argv);
#endif // DISABLE_MALLOC
	setConsoleColors(getDefaultColors());
	return WALLSHELL_NO_ERROR;
}

// Default prefix
const char* prefix = "> ";
void setConsolePrefix(const char* newPrefix) { prefix = newPrefix; }

wallshell_error_t terminalMain() {
	/* We're assuming that the user has printed everything they want prior to calling main. */
	/* We're also assuming the colors have been defined, even if they are blank. */
#ifndef NO_BASIC_COMMANDS
	registerBasicCommands();
#endif
	
	// Check for stream configurations
	if (!wallshell_err_stream) setStream(WALLSHELL_ERROR, stderr);
	if (!wallshell_out_stream) setStream(WALLSHELL_OUTPUT, stdout);
	if (!wallshell_in_stream) setStream(WALLSHELL_INPUT, stdin);

#ifndef CUSTOM_CONSOLE_SETUP
	setConsoleMode();
#endif // CUSTOM_CONSOLE_SETUP
	
	// Make sure the colors are set properly if they are defaults
	updateColors();
	
	/* Ideally something should've caught this before calling main, but we still need to check. */
#ifndef DISABLE_MALLOC
	if (!commands) commands = malloc(sizeof(command_t));
	if (!commands) return WALLSHELL_OUT_OF_MEMORY;
#endif
	bool newCommand = true;
	bool tabPressed = false; // allows for autocompletion
	
	size_t position_in_previous = 0;
	size_t current_position = 1;
	
	char commandBuf[MAX_COMMAND_BUF];
	char oldCommand[MAX_COMMAND_BUF];
	
	input_result_t input_result = {0, 0};
	
	while (!exit_terminal) {
		if (newCommand) {
			fprintf(wallshell_out_stream, "%s", prefix);
			newCommand = false;
			tabPressed = false;
			position_in_previous = 0;
			current_position = 1;
			memset(oldCommand, 0, MAX_COMMAND_BUF);
			memset(commandBuf, 0, MAX_COMMAND_BUF);
		}
		
		// Check for the previous input results
		if (input_result.type != NONE) {
			if (input_result.type == CURSOR) {
				switch (input_result.result) {
					case CONSOLE_CURSOR_UP: {
						CLEAR_ROW;
						if (position_in_previous == 0) {
							memset(oldCommand, 0, MAX_COMMAND_BUF);
							memcpy(oldCommand, commandBuf, MAX_COMMAND_BUF);
						}
						memset(commandBuf, 0, MAX_COMMAND_BUF);
						memcpy(commandBuf, previousCommands[position_in_previous], strlen(previousCommands[position_in_previous]));
						fprintf(wallshell_out_stream, "\r%s%s", prefix, commandBuf);
						if (previous_commands_size > 0 && position_in_previous < previous_commands_size - 1) {
							position_in_previous++;
						}
						input_result.type = NONE;
						current_position = 1;
						continue;
					}
					case CONSOLE_CURSOR_DOWN: {
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
						fprintf(wallshell_out_stream, "\r%s%s", prefix, commandBuf);
						current_position = 1;
						input_result.type = NONE;
						continue;
					}
					case CONSOLE_CURSOR_RIGHT: {
						if (current_position == (strlen(commandBuf) + 1)) break;
						current_position++;
						wallshell_move_cursor(CONSOLE_CURSOR_RIGHT);
						input_result.type = NONE;
						continue;
					}
					case CONSOLE_CURSOR_LEFT: {
						if (current_position == 1) break;
						current_position--;
						wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
						input_result.type = NONE;
						continue;
					}
					default: break;
				}
			}
		}
		
		int current = wallshell_get_char(wallshell_in_stream);
		if (backspace_as_ascii_delete && current == 0x7f)
			current = '\b';
		if (current == '\n' || current == '\r') {
			// If there's an empty command we just start a new line.
			fprintf(wallshell_out_stream, "\n");
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
			executeCommand(commandBuf);
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
					fprintf(wallshell_out_stream, "%s%s", prefix, commandBuf);
					wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
					for (size_t i = strlen(commandBuf); i > current_position; i--) {
						wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
					}
				} else {
					// only clear the last char. much quicker than rewriting the line
					wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
					fprintf(wallshell_out_stream, " ");
					wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
				}
			}
		} else if (current == '\t') {
			// see if we can autocomplete a command.
			const char* list[50]; // List of current possible commands
			int list_size = 0;
			for (int i = 0; i < command_size; i++) {
				setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_GREEN, CONSOLE_BG_DEFAULT});
				if (commands[i].commandName && wallshell_internal_startsWith(commands[i].commandName, commandBuf)) {
					list[list_size] = commands[i].commandName;
					list_size++;
				}
				
				// Check aliases for a match
				for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
					if (commands[i].aliases[alias_idx] && wallshell_internal_startsWith(commands[i].aliases[alias_idx], commandBuf)) {
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
				setConsoleColors(getDefaultColors());
			}
			
			if (list_size == 1) {
				// Print the rest of the command
				size_t len = strlen(commandBuf);
				const char* currentCommand = list[0];
				for (size_t i = len; i < strlen(currentCommand); i++) {
					fprintf(wallshell_out_stream, "%c", currentCommand[i]);
					wallshell_internal_strcat_c(commandBuf, currentCommand[i], MAX_COMMAND_BUF);
				}
				tabPressed = false;
			} else if (tabPressed) {
				if (list_size == 0) {
					setConsoleColors((console_color_t) {CONSOLE_FG_BRIGHT_RED, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "\nNo command starting with: %s\n", commandBuf);
					// Clear the buffer
					memset(commandBuf, 0, MAX_COMMAND_BUF * sizeof(char));
					commandBuf[0] = '\0';
					newCommand = true;
				} else if (list_size > 1) {
					// Print out all commands
					setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
					fprintf(wallshell_out_stream, "\n");
					for (int i = 0; i < list_size; i++) {
						fprintf(wallshell_out_stream, "%s\n", list[i]);
					}
					setConsoleColors(getDefaultColors());
					// Reprint the command line
					fprintf(wallshell_out_stream, "\r%s%s", prefix, commandBuf);
				}
				tabPressed = false;
			} else {
				tabPressed = true;
			}
			setConsoleColors(getDefaultColors());
		} else if (current == EOF) {
			// Temporarily for development’s sake, this is how you exit the console.
			// ctrl+d on unix, ctrl+z on windows
			break;
		} else if (current == '\033') {
			input_result = processVirtualSequence();
		} else if (current == 0xE0) {
			// Microsoft sometimes wants to work with virtual inputs but usually doesn't.
			// At the very least this makes porting it to an os very easy.
			// All the OS has to do is give this program raw input in the form of scancodes for special keys.
			input_result = processEO();
		} else {
			
			wallshell_internal_insert_c(commandBuf, MAX_COMMAND_BUF, (char) current, current_position);
			if (current_position != strlen(commandBuf)) {
				CLEAR_ROW;
				fprintf(wallshell_out_stream, "%s%s", prefix, commandBuf);
				for (size_t i = strlen(commandBuf); i > current_position; i--) {
					wallshell_move_cursor(CONSOLE_CURSOR_LEFT);
				}
			} else {
				fprintf(wallshell_out_stream, "%c", current);
			}
			current_position++;
			//printf("current command: %s -> size: %llu -> pos: %zu\n", commandBuf, strlen(commandBuf), current_position);
		}
	}
#ifndef CUSTOM_CONSOLE_SETUP
	resetConsoleState();
#endif // CUSTOM_CONSOLE_SETUP
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
bool compareCommands(const command_t c1, const command_t c2) {
	if (c1.aliases != c2.aliases) return false;
	if (c1.aliases_count != c2.aliases_count) return false;
	if (strcmp(c1.commandName, c2.commandName) != 0) return false;
	if (c1.helpCommand != c2.helpCommand) return false;
	if (c1.mainCommand != c2.mainCommand) return false;
	return true;
}

/**
 * @brief Sets the console locale. This is only required on Windows systems, as terminals default to ASCII.
 * The built in SET_TERMINAL_LOCALE sets the Windows terminal to UTF8.
 * Can be potentially be used on unix systems to configure locale, although this isn't done by default.
 * If the system you are implementing requires locale configuration, redefine SET_TERMINAL_LOCALE to the needed configuration.
 */
void setConsoleLocale() { SET_TERMINAL_LOCALE; }

void printGeneralHelp(help_entry_general_t* entry) {
	// Command Name
	setConsoleColors((console_color_t) {CONSOLE_FG_RED, CONSOLE_BG_DEFAULT});
	if (entry->commandName)
		fprintf(wallshell_out_stream, "\n%s\n", entry->commandName);
	
	// Description
	setConsoleColors((console_color_t) {CONSOLE_FG_CYAN, CONSOLE_BG_DEFAULT});
	if (entry->description)
		fprintf(wallshell_out_stream, "%s\n", entry->description);
	
	// Commands
	if (entry->commands_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "\nCommands:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->commands_count; i++) {
			if (entry->commands[i])
				fprintf(wallshell_out_stream, "  %s\n", entry->commands[i]);
		}
	}
	
	// Aliases
	if (entry->aliases_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "\nAliases:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->aliases_count; i++) {
			if (entry->aliases[i])
				fprintf(wallshell_out_stream, "  %s\n", entry->aliases[i]);
		}
	}
	setConsoleColors((console_color_t) {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT});
	fprintf(wallshell_out_stream, "\n");
}

void printSpecificHelp(help_entry_specific_t* entry) {
	// Command Name
	setConsoleColors((console_color_t) {CONSOLE_FG_RED, CONSOLE_BG_DEFAULT});
	if (entry->commandName)
		fprintf(wallshell_out_stream, "\n%s\n", entry->commandName);
	
	// Description
	setConsoleColors((console_color_t) {CONSOLE_FG_CYAN, CONSOLE_BG_DEFAULT});
	if (entry->description)
		fprintf(wallshell_out_stream, "%s\n", entry->description);
	
	// Commands
	if (entry->required_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "Required:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->required_count; i++) {
			if (entry->required[i])
				fprintf(wallshell_out_stream, "  %s\n", entry->required[i]);
		}
	}
	
	// Aliases
	if (entry->optional_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		fprintf(wallshell_out_stream, "\nOptional:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->optional_count; i++) {
			if (entry->optional[i])
				fprintf(wallshell_out_stream, "  %s\n", entry->optional[i]);
		}
	}
	setConsoleColors((console_color_t) {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT});
	fprintf(wallshell_out_stream, "\n");
}

#include <stdarg.h>
bool promptUser(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	vfprintf(wallshell_out_stream, format, arg);
	va_end(arg);
	
	fprintf(wallshell_out_stream, " [Y/n] ");
	int first_input = wallshell_get_char(wallshell_in_stream);
	fprintf(wallshell_out_stream, "%c", first_input);
	int input;
	do {
		input = wallshell_get_char(wallshell_in_stream);
		fprintf(wallshell_out_stream, "%c", input);
	} while (input != '\n');
	if (first_input == 'Y' || first_input == 'y') return true;
	return false;
}