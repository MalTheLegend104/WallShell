/**
 * @file main.c
 * @author MalTheLegend104
 * @brief Example 1
 *
 * This file is the first example of how to use WallShell in your program.
 *
 * @version v1.0.0
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
#include "../wall_shell.h"


/**************************************************************************
 * This example works for any combination of compile time flags.
 * This will work for both DISABLE_MALLOC and THREADED_SUPPORT
 *************************************************************************/
int example(int argc, char** argv) {
	if (argc > 1) {
		// There's always at least 1 argument. argv[0] is *always* the command name (or an alias).
		// For this example, we're just going to log the arguments and their numbers.
		for (int i = 1; i < argc; i++) {
			ws_logger(WS_LOG, "Argument %d: \"%s\"", i, argv[i]);
		}
	} else {
		ws_logger(WS_ERROR, "No arguments provided");
		return -1;
	}
	return 0;
}

// For this example, we define the aliases in a global scope.
// You can malloc this and pass it when registering the command if you wish.
// When using DISABLE_MALLOC, defining it in this scope is required.
const char* example2_aliases[] = { "ex2", "exam2" };
int example2(int argc, char** argv) {
	if (argc > 1) {
		// This command has a flag: -l
		// We're going to make the -l flag log the next argument
		if (strcmp(argv[1], "-l") == 0) {
			if (argc >= 2) ws_logger(WS_INFO, "%s", argv[2]);
			else ws_logger(WS_WARN, "Flag \"-l\" requires an additional argument.");
		} else {
			ws_logger(WS_ERROR, "Unrecognized flag: \"%s\"", argv[1]);
			return -1;
		}
	} else {
		ws_setForegroundColor(WS_FG_BRIGHT_CYAN);
		printf("This command requires a flag. Type \"help example2\" to learn more.\n");
		ws_setConsoleColors(ws_getDefaultColors());
	}
	return 0;
}

int example2_help(int argc, char** argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "-l") == 0) {
			const char* required[] = {
				"<arg> - This command must be followed by an argument to log",
			};
			ws_help_entry_specific_t help = {
				"Example 2 - \"Log\"",
				"Example 2 log command, it logs the next argument in the command string.",
				required,
				1, // There is one required arg.
				NULL, // No optional args
				0 // No optional args
			};

			ws_printSpecificHelp(&help);
			return 0;
		}
	}

	const char* commands[] = {
		"-l <arg> -> Logs the provided argument.",
	};
	ws_help_entry_general_t gen_help = {
		"Example 2",
		"The second example command.",
		commands,
		1,
		example2_aliases,
		2
	};
	ws_printGeneralHelp(&gen_help);
	return 0;
}

int main() {
#ifdef THREADED_SUPPORT
	// This is a single threaded application, we dont need the threadID.
	ws_doPrintThreadID(false);
#endif

	// Changes the prefix from "> " to "$ "
	ws_setConsolePrefix("$ ");

	// You might have to set the locale on some platforms
	// This sets it to UTF-8 on windows, and does nothing on POSIX
	// It doesn't hurt to call it regardless, it does nothing on platforms where it's not needed.
	ws_setConsoleLocale();

	// Here, we create the command "example", with no help function and no aliases
	ws_registerCommand((ws_command_t) { example, NULL, "example", NULL, 0 });

	// This one, we'll create "example2", that has a help command and aliases
	// You could also define the aliases here with malloc/calloc, rather than in a global scope.
	ws_registerCommand((ws_command_t) { example2, example2_help, "example2", example2_aliases, 2 });

	// Main terminal function. Always call this last.
	ws_terminalMain();

	// Always make sure to clean up the terminal.
	// Always call this before exiting.
	ws_cleanAll();
}