/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */
#include "wall_shell.h"
#ifdef _WIN32
	#include <Windows.h>
#endif

// Thank you  microsoft for making my life harder
#ifdef _MSC_VER
// Disables the warning for string.h "deprecated" functions
// We're in C99, and the stuff we're touching should either be manually mutex locked or single threaded anyway.
// Commands that are outside the scope of this file should be properly mutex locked if they touch things running in other threads.
#pragma warning(disable : 4996)
#endif

// For some systems (mostly POSIX), backspace gets sent as ascii delete rather than \b
bool backspace_as_ascii_delete = false;

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

// ------------------------------------------------------------------------------------------------
// Streams
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

// ------------------------------------------------------------------------------------------------
// Default console color output.
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
	fprintf(wallshell_out_stream, "\033[%d;%dm", fg, bg);
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

char previousCommands[PREVIOUS_BUF_SIZE][MAX_COMMAND_BUF];
size_t previous_commands_size;

// ------------------------------------------------------------------------------------------------
// Console Color Configuration
// ------------------------------------------------------------------------------------------------
console_color_t default_colors = {CONSOLE_FG_BRIGHT_WHITE, CONSOLE_BG_BLACK};
console_color_t current_colors = {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT};

wallshell_error_t updateColors() {
	if (!wallshell_out_stream) return WALLSHELL_OUT_STREAM_NOT_SET;
	if (current_colors.foreground == CONSOLE_FG_DEFAULT) {
		// It's guaranteed that the user cant set default_colors.foreground to CONSOLE_FG_DEFAULT
		current_colors.foreground = default_colors.foreground;
	}
	if (current_colors.background == CONSOLE_BG_DEFAULT) {
		current_colors.background = default_colors.background;
	}
	SET_CONSOLE_COLORS(current_colors.foreground, current_colors.background);
	return WALLSHELL_NO_ERROR;
}

wallshell_error_t setConsoleForegroundDefault(console_fg_color_t c) {
	if (c == CONSOLE_FG_DEFAULT) return WALLSHELL_CANT_SET_DEFAULT_TO_DEFAULT;
	default_colors.foreground = c;
	return WALLSHELL_NO_ERROR;
}
wallshell_error_t setConsoleBackgroundDefault(console_bg_color_t c) {
	if (c == CONSOLE_BG_DEFAULT) return WALLSHELL_CANT_SET_DEFAULT_TO_DEFAULT;
	default_colors.background = c;
	return WALLSHELL_NO_ERROR;
}
wallshell_error_t setConsoleDefaults(console_color_t c) {
	wallshell_error_t ret;
	ret = setConsoleForegroundDefault(c.foreground);
	if (ret == WALLSHELL_NO_ERROR)
		ret = setConsoleBackgroundDefault(c.background);
	return ret;
}

console_color_t getCurrentColors() { return current_colors; }
console_color_t getDefaultColors() { return default_colors; }

wallshell_error_t setConsoleColors(console_color_t colors) {
	current_colors.foreground = colors.foreground;
	current_colors.background = colors.background;
	return updateColors();
}
wallshell_error_t setForegroundColor(console_fg_color_t color) {
	if (current_colors.foreground == CONSOLE_FG_DEFAULT)
		current_colors.foreground = default_colors.foreground;
	else current_colors.foreground = color;
	return updateColors();
}
wallshell_error_t setBackgroundColor(console_bg_color_t color) {
	if (current_colors.background == CONSOLE_BG_DEFAULT)
		current_colors.background = default_colors.background;
	else current_colors.background = color;
	return updateColors();
}

/**
 * @brief Register the command to the command handler.
 * @param c Command to be registered.
 */
wallshell_error_t registerCommand(const command_t c) {
#ifdef DISABLE_MALLOC
	if (current_command_spot != COMMAND_LIMIT){
		commands[current_command_spot] = c;
		current_command_spot++;
	} else {
		return WALLSHELL_COMMAND_LIMIT_REACHED;
	}
#else
	if (!commands) {
		commands = malloc(sizeof(command_t));
		command_size = 1;
	} else if (current_command_spot > command_size) {
		// realloc invalidates the old pointer on call, but leaves it alone if it cant find the memory.
		// If this function returns with an out of memory error, the shell is still usable.
		command_t* new_ptr = realloc(commands, (size_t) ((double) command_size * sizeof(command_t) * 1.5));
		if (!new_ptr) return WALLSHELL_OUT_OF_MEMORY;
		else commands = new_ptr;
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

void registerBasicCommands() {
	// a bare shell only has help, exit, clear, and history
	// might come up with some more overtime, such as echo, but it's not a big priority.
	command_t c = {test, NULL, "test", NULL, 0};
	registerCommand(c);
}

#ifdef DISABLE_MALLOC
wallshell_error_t executeCommand(char* commandBuf) {
//  int argc = 0;
//	char argv[MAX_ARGS][MAX_COMMAND_BUF];
	return WALLSHELL_NO_ERROR;
}
#else
wallshell_error_t executeCommand(char* commandBuf) {
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
		argv[argc] = strdup(current);
		current = strtok(NULL, " ");
		argc++;
	}
	if (argc == 0) {
		// Somehow got an empty command.
		if (argv) free(argv);
		return WALLSHELL_NO_ERROR;
	}
	
	// Call Command (if it exists)
	for (size_t i = 0; i < command_size; i++) {
		if (commands[i].commandName && strcmp(commands[i].commandName, argv[0]) == 0) {
			int result = commands[i].mainCommand(argc, argv);
			if (result != 0) {
				// If the command function returns a non-zero value, it may indicate an error
				printf("Command exited with code: %d\n", result);
			}
			goto cleanup;
		}
		
		// Check that commands alias
		for (size_t alias_idx = 0; alias_idx < commands[i].aliases_count; alias_idx++) {
			if (commands[i].aliases[alias_idx] && strcmp(commands[i].aliases[alias_idx], argv[0]) == 0) {
				int result = commands[i].mainCommand(argc, argv);
				if (result != 0) {
					// If the command function returns a non-zero value, it may indicate an error
					printf("Command exited with code: %d\n", result);
				}
				goto cleanup;
			}
		}
	}
	printf("Command not found: \"%s\"\n", argv[0]);
cleanup:
	for (int i = 0; i < argc; i++) free(argv[i]);
	free(argv);
	return WALLSHELL_NO_ERROR;
}
#endif // DISABLE_MALLOC

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
	if (!commands) commands = malloc(sizeof(command_t));
	if (!commands) return WALLSHELL_OUT_OF_MEMORY;
	
	bool newCommand = true;
	// bool tabPressed = false; // allows for autocompletion
	
	// size_t position_in_previous = 0;
	
	char commandBuf[MAX_COMMAND_BUF];
	char oldCommand[MAX_COMMAND_BUF];
	
	while (true) {
		if (newCommand) {
			fprintf(wallshell_out_stream, "%s", prefix);
			newCommand = false;
			// tabPressed = false;
			// position_in_previous = 0;
			memset(oldCommand, 0, MAX_COMMAND_BUF);
			memset(commandBuf, 0, MAX_COMMAND_BUF);
		}
		
		int current = wallshell_get_char(wallshell_in_stream);
		if (backspace_as_ascii_delete && current == 0x7f)
			current = '\b';
		//printf("%c - %d\n", current, current); // useful for debugging your wallshell_get_char
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
				// Remove the last character by setting it to null terminator
				commandBuf[strlen(commandBuf) - 1] = '\0';
				// We also need to clear the character from the terminal. This is a little cursed.
				// It uses delete to move the cursor back one, prints a space to make sure it's cleared, the goes back one again.
				// If I implement a better cursor system this will likely get changed later.
				
				// Expected behavior is backspace moves the cursor back one, prints a space, then moves it back again.
				fprintf(wallshell_out_stream, "\b \b");
			}
		} else if (current == '\t') {
			// see if we can autocomplete a command.
			// TODO implement this feature.
		} else if (current == EOF) {
			// Temporarily for development’s sake, this is how you exit the console.
			// ctrl+d on unix, ctrl+z on windows
			break;
			//} else if (current == '\r') {
			//	enterPressed = true;
		} else {
			fprintf(wallshell_out_stream, "%c", current);
			wallshell_internal_strcat_c(commandBuf, (char) current, MAX_COMMAND_BUF);
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
		printf("\n%s\n", entry->commandName);
	
	// Description
	setConsoleColors((console_color_t) {CONSOLE_FG_CYAN, CONSOLE_BG_DEFAULT});
	if (entry->description)
		printf("%s\n", entry->description);
	
	// Commands
	if (entry->required_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		printf("Required:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->required_count; i++) {
			if (entry->required[i])
				printf("  %s\n", entry->required[i]);
		}
	}
	
	// Aliases
	if (entry->optional_count > 0) {
		setConsoleColors((console_color_t) {CONSOLE_FG_YELLOW, CONSOLE_BG_DEFAULT});
		printf("\nOptional:\n");
		
		setConsoleColors((console_color_t) {CONSOLE_FG_GREEN, CONSOLE_BG_DEFAULT});
		for (int i = 0; i < entry->optional_count; i++) {
			if (entry->optional[i])
				printf("  %s\n", entry->optional[i]);
		}
	}
	setConsoleColors((console_color_t) {CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT});
	printf("\n");
}