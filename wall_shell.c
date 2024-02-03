/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */
#include "wall_shell.h"

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
// Streams, perhaps the most important thing for this entire file.
// ------------------------------------------------------------------------------------------------
FILE* wallshell_out_stream = NULL;
FILE* wallshell_err_stream = NULL;
FILE* wallshell_in_stream = NULL;

void setStream(wallshell_stream type, FILE* stream) {
	switch (type) {
		case WALLSHELL_INPUT:
			wallshell_in_stream = stream;
			break;
		case WALLSHELL_OUTPUT:
			wallshell_out_stream = stream;
			break;
		case WALLSHELL_ERROR:
			wallshell_err_stream = stream;
			break;
		default: break;
	}
}

void initalizeDefaultStreams() {
	setStream(WALLSHELL_INPUT, stdin);
	setStream(WALLSHELL_ERROR, stderr);
	setStream(WALLSHELL_OUTPUT, stdout);
}

// ------------------------------------------------------------------------------------------------
// Default console color output.
// ------------------------------------------------------------------------------------------------
#ifndef CUSTOM_CONSOLE_COLORS
	#ifdef _WIN32
		/* I'm trying to include the least amount of windows headers as possible */
		#include <Windows.h>
		#define SET_TERMINAL_LOCALE    SetConsoleOutputCP(CP_UTF8)
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		#define SET_TERMINAL_LOCALE
	#endif
	/* Modern windows supports these escape codes, older windows versions use SetConsoleTextAttribute */
	/* https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences */
	#define RESET_CONSOLE                   fprintf(wallshell_out_stream, "\e[27m");
	
	#define SET_CONSOLE_FG_BLACK            fprintf(wallshell_out_stream, "\e[30m");
	#define SET_CONSOLE_FG_BRIGHT_BLACK     fprintf(wallshell_out_stream, "\e[90m");
	#define SET_CONSOLE_FG_WHITE            fprintf(wallshell_out_stream, "\e[37m");
	#define SET_CONSOLE_FG_BRIGHT_WHITE     fprintf(wallshell_out_stream, "\e[97m");
	#define SET_CONSOLE_FG_RED              fprintf(wallshell_out_stream, "\e[31m");
	#define SET_CONSOLE_FG_BRIGHT_RED       fprintf(wallshell_out_stream, "\e[91m");
	#define SET_CONSOLE_FG_GREEN            fprintf(wallshell_out_stream, "\e[32m");
	#define SET_CONSOLE_FG_BRIGHT_GREEN     fprintf(wallshell_out_stream, "\e[92m");
	#define SET_CONSOLE_FG_YELLOW           fprintf(wallshell_out_stream, "\e[32m");
	#define SET_CONSOLE_FG_BRIGHT_YELLOW    fprintf(wallshell_out_stream, "\e[93m");
	#define SET_CONSOLE_FG_CYAN             fprintf(wallshell_out_stream, "\e[36m");
	#define SET_CONSOLE_FG_BRIGHT_CYAN      fprintf(wallshell_out_stream, "\e[96m");
	#define SET_CONSOLE_FG_BLUE             fprintf(wallshell_out_stream, "\e[34m");
	#define SET_CONSOLE_FG_BRIGHT_BLUE      fprintf(wallshell_out_stream, "\e[94m");
	#define SET_CONSOLE_FG_MAGENTA          fprintf(wallshell_out_stream, "\e[35m");
	#define SET_CONSOLE_FG_BRIGHT_MAGENTA   fprintf(wallshell_out_stream, "\e[95m");
	
	#define SET_CONSOLE_BG_BLACK            fprintf(wallshell_out_stream, "\e[40m");
	#define SET_CONSOLE_BG_BRIGHT_BLACK     fprintf(wallshell_out_stream, "\e[100m");
	#define SET_CONSOLE_BG_WHITE            fprintf(wallshell_out_stream, "\e[47m");
	#define SET_CONSOLE_BG_BRIGHT_WHITE     fprintf(wallshell_out_stream, "\e[107m");
	#define SET_CONSOLE_BG_RED              fprintf(wallshell_out_stream, "\e[41m");
	#define SET_CONSOLE_BG_BRIGHT_RED       fprintf(wallshell_out_stream, "\e[101m");
	#define SET_CONSOLE_BG_GREEN            fprintf(wallshell_out_stream, "\e[42m");
	#define SET_CONSOLE_BG_BRIGHT_GREEN     fprintf(wallshell_out_stream, "\e[102m");
	#define SET_CONSOLE_BG_YELLOW           fprintf(wallshell_out_stream, "\e[42m");
	#define SET_CONSOLE_BG_BRIGHT_YELLOW    fprintf(wallshell_out_stream, "\e[103m");
	#define SET_CONSOLE_BG_CYAN             fprintf(wallshell_out_stream, "\e[46m");
	#define SET_CONSOLE_BG_BRIGHT_CYAN      fprintf(wallshell_out_stream, "\e[106m");
	#define SET_CONSOLE_BG_BLUE             fprintf(wallshell_out_stream, "\e[44m");
	#define SET_CONSOLE_BG_BRIGHT_BLUE      fprintf(wallshell_out_stream, "\e[104m");
	#define SET_CONSOLE_BG_MAGENTA          fprintf(wallshell_out_stream, "\e[45m");
	#define SET_CONSOLE_BG_BRIGHT_MAGENTA   fprintf(wallshell_out_stream, "\e[105m");
#endif // CUSTOM_CONSOLE_COLORS

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Buffer declarations. Also checks for malloc usage to configure the buffers.
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#ifdef DISABLE_MALLOC
command_t commands[COMMAND_LIMIT];
size_t command_size = COMMAND_LIMIT;
	#define MALLOC_DISABLED true
#else
command_t* commands;
size_t command_size = 0;
	#define MALLOC_DISABLED false
#endif

size_t current_command_spot = 0;

char previousCommands[PREVIOUS_BUF_SIZE][MAX_COMMAND_BUF];
size_t previous_commands_size;

// ------------------------------------------------------------------------------------------------
// Console Color Configuration
// ------------------------------------------------------------------------------------------------
console_color_t default_colors = { CONSOLE_FG_BRIGHT_WHITE, CONSOLE_BG_BLACK };
console_color_t current_colors = { CONSOLE_FG_DEFAULT, CONSOLE_BG_DEFAULT };

wallshell_error_t updateFG() {
	if (!wallshell_out_stream) return WALLSHELL_OUT_STREAM_NOT_SET;
	if (current_colors.foreground == CONSOLE_FG_DEFAULT) {
		// It's guaranteed that the user cant set default_colors.foreground to CONSOLE_FG_DEFAULT
		current_colors.foreground = default_colors.foreground;
		return updateFG();
	}
	switch (current_colors.foreground) {
		case CONSOLE_FG_BLACK: SET_CONSOLE_FG_BLACK; break;
		case CONSOLE_FG_BRIGHT_BLACK: SET_CONSOLE_FG_BRIGHT_BLACK; break;
		case CONSOLE_FG_WHITE: SET_CONSOLE_FG_WHITE; break;
		case CONSOLE_FG_BRIGHT_WHITE: SET_CONSOLE_FG_BRIGHT_WHITE; break;
		case CONSOLE_FG_RED: SET_CONSOLE_FG_RED; break;
		case CONSOLE_FG_BRIGHT_RED: SET_CONSOLE_FG_BRIGHT_RED; break;
		case CONSOLE_FG_GREEN: SET_CONSOLE_FG_GREEN; break;
		case CONSOLE_FG_BRIGHT_GREEN: SET_CONSOLE_FG_BRIGHT_GREEN; break;
		case CONSOLE_FG_YELLOW: SET_CONSOLE_FG_YELLOW; break;
		case CONSOLE_FG_BRIGHT_YELLOW: SET_CONSOLE_FG_BRIGHT_YELLOW; break;
		case CONSOLE_FG_CYAN: SET_CONSOLE_FG_CYAN; break;
		case CONSOLE_FG_BRIGHT_CYAN: SET_CONSOLE_FG_BRIGHT_CYAN; break;
		case CONSOLE_FG_BLUE: SET_CONSOLE_FG_BLUE; break;
		case CONSOLE_FG_BRIGHT_BLUE: SET_CONSOLE_FG_BRIGHT_BLUE; break;
		case CONSOLE_FG_MAGENTA: SET_CONSOLE_FG_MAGENTA; break;
		case CONSOLE_FG_BRIGHT_MAGENTA: SET_CONSOLE_FG_BRIGHT_MAGENTA; break;
		default: break;
	}
	return WALLSHELL_NO_ERROR;
}

wallshell_error_t updateBG() {
	if (!wallshell_out_stream) return WALLSHELL_OUT_STREAM_NOT_SET;
	if (current_colors.background == CONSOLE_BG_DEFAULT) {
		// It's guaranteed that the user cant set default_colors.background to CONSOLE_BG_DEFAULT
		current_colors.background = default_colors.background;
		return updateFG();
	}
	switch (current_colors.background){
		case CONSOLE_BG_BLACK: SET_CONSOLE_BG_BLACK; break;
		case CONSOLE_BG_BRIGHT_BLACK: SET_CONSOLE_BG_BRIGHT_BLACK; break;
		case CONSOLE_BG_WHITE: SET_CONSOLE_BG_WHITE; break;
		case CONSOLE_BG_BRIGHT_WHITE: SET_CONSOLE_BG_BRIGHT_WHITE; break;
		case CONSOLE_BG_RED: SET_CONSOLE_BG_RED; break;
		case CONSOLE_BG_BRIGHT_RED: SET_CONSOLE_BG_BRIGHT_RED; break;
		case CONSOLE_BG_GREEN: SET_CONSOLE_BG_GREEN; break;
		case CONSOLE_BG_BRIGHT_GREEN: SET_CONSOLE_BG_BRIGHT_GREEN; break;
		case CONSOLE_BG_YELLOW: SET_CONSOLE_BG_YELLOW; break;
		case CONSOLE_BG_BRIGHT_YELLOW: SET_CONSOLE_BG_BRIGHT_YELLOW; break;
		case CONSOLE_BG_CYAN: SET_CONSOLE_BG_CYAN; break;
		case CONSOLE_BG_BRIGHT_CYAN: SET_CONSOLE_BG_BRIGHT_CYAN; break;
		case CONSOLE_BG_BLUE: SET_CONSOLE_BG_BLUE; break;
		case CONSOLE_BG_BRIGHT_BLUE: SET_CONSOLE_BG_BRIGHT_BLUE; break;
		case CONSOLE_BG_MAGENTA: SET_CONSOLE_BG_MAGENTA; break;
		case CONSOLE_BG_BRIGHT_MAGENTA: SET_CONSOLE_BG_BRIGHT_MAGENTA; break;
		default: break;
	}
	return WALLSHELL_NO_ERROR;
}

wallshell_error_t updateColors() {
	wallshell_error_t ret = updateFG();
	if (!ret) ret = updateBG();
	return ret;
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

console_color_t getCurrentColors(){ return current_colors; }
console_color_t getDefaultColors(){ return default_colors; }

wallshell_error_t setConsoleColors(console_color_t colors) {
	current_colors.foreground = colors.foreground;
	current_colors.background = colors.background;
	return updateColors();
}
wallshell_error_t setForegroundColor(console_fg_color_t color){
	if (current_colors.foreground == CONSOLE_FG_DEFAULT)
		current_colors.foreground = default_colors.foreground;
	else current_colors.foreground = color;
	return updateFG();
}
wallshell_error_t setBackgroundColor(console_bg_color_t color){
	if (current_colors.background == CONSOLE_BG_DEFAULT)
		current_colors.background = default_colors.background;
	else current_colors.background = color;
	return updateBG();
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
		if (!commands)
			commands = malloc(sizeof(command_t));
		else if (current_command_spot > command_size) {
			// realloc invalidates the old pointer on call, but leaves it alone if it cant find the memory.
			// If this function returns with an out of memory error, the shell is still usable.
			command_t* new_ptr = realloc(commands, (size_t) (command_size * sizeof(command_t) * 1.5));
			if (!new_ptr) return WALLSHELL_OUT_OF_MEMORY;
			else commands = new_ptr;
		}
		commands[current_command_spot] = c;
#endif
	return WALLSHELL_NO_ERROR;
}

void deregisterCommand(const command_t c) {

}

void executeCommand(char* commandBuf) {
	fprintf(wallshell_out_stream, "Command buf: %s", commandBuf);
}

void registerBasicCommands(){
	// a bare shell only has help, exit, clear, and history
	// might come up with some more overtime, such as echo, but it's not a big priority.
}

wallshell_error_t terminalMain() {
	/* We're assuming that the user has printed everything they want prior to calling main. */
	/* We're also assuming the colors have been defined, even if they are blank. */
#ifndef NO_BASIC_COMMMANDS
	registerBasicCommands();
#endif
	
	// Check for stream configurations
	if (!wallshell_err_stream) setStream(WALLSHELL_ERROR, stderr);
	if (!wallshell_out_stream) setStream(WALLSHELL_OUTPUT, stdout);
	if (!wallshell_in_stream) setStream(WALLSHELL_INPUT, stdin);
	
	// Make sure the colors are set properly if they are defaults
	if (current_colors.foreground == CONSOLE_FG_DEFAULT) updateFG();
	if (current_colors.background == CONSOLE_BG_DEFAULT) updateBG();
	
	/* Ideally something should've caught this before calling main, but we still need to check. */
	if (!commands) commands = malloc(sizeof(command_t));
	if (!commands) return WALLSHELL_OUT_OF_MEMORY;
	
	
	bool newCommand = true;
	bool tabPressed = false; // allows for autocompletion
	
	size_t position_in_previous = 0;
	
	char commandBuf[MAX_COMMAND_BUF];
	char oldCommand[MAX_COMMAND_BUF];

	while (true){
		if (newCommand){
			fprintf(wallshell_out_stream, "\n%s ", SHELL_COMMAND_PREFIX);
			newCommand = false;
			tabPressed = false;
			position_in_previous = 0;
			memset(oldCommand, 0, MAX_COMMAND_BUF);
			memset(commandBuf, 0, MAX_COMMAND_BUF);
		}
		
		int current = fgetc(wallshell_in_stream);
		if (current == '\n') {
			// If there's an empty command we just start a new line.
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
				printf("%c", current);
				commandBuf[strlen(commandBuf) - 1] = '\0';
				// We also need to clear the character from the terminal. This is a little cursed.
				// It uses delete to move the cursor back one, prints a space to make sure it's cleared, the goes back one again.
				// If I implement a better cursor system this will likely get changed later.
				fprintf(wallshell_out_stream, "\b \b");
			}
		} else if (current == '\t') {
			// see if we can autocomplete a command.
			// TODO implement this feature.
		} else if (current == EOF) {
			// Temporarily for development’s sake, this is how you exit the console.
			// ctrl+d on unix, ctrl+z on windows
			break;
		} else {
			wallshell_internal_strcat_c(commandBuf,(char) current, MAX_COMMAND_BUF);
		}
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

}

void printSpecificHelp(help_entry_specific_t* entry) {

}