/**
 * @file main.c
 * @author MalTheLegend104
 * @brief Example 1
 *
 * This file is the first example of how to use WallShell in your program.
 *
 * @version v2.0.0
 * @copyright
 * Copyright 2026 MalTheLegend104
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

// ------------------------------------------------------------------------------------------------
// Defines argument rules for "example"
// ------------------------------------------------------------------------------------------------

static const ws_command_argument_t example_args[] = {
	// Creates a required generic argument (no --) 
	{
		.type = WS_ARG_TYPE_GENERIC,
		.required = true,
		.name = "arg",
		.shortform = NULL,
		.description = "The argument to log"
	}
	/* There is no need for a --help or -h argument. WallShell will automatically deal with those. */
};

int example(int argc, char** argv) {
	// WallShell stores the context for the command internally.
	// If you want to use the argument parsing, you need to use this.
	// WallShell will only allow one command to invoked at a time, regardless of threaded support or not.
	// This context *should not* be used outside the function called by WallShell.
	// If you need information from this, you should copy it.
	ws_context_t* ctx = ws_getCurrentContext();

	if (!ws_parse_args(ctx, argc, argv)) {
		// ws_parse_args will print an error on an invalid parse 
		return -1;
	}

	const char* arg_val = ws_get_generic(ctx, "arg");
	if (arg_val) {
		ws_logger(WS_LOG, "Argument 1: \"%s\"", arg_val);
	}

	return 0;
}

/* WallShell will handle help for "example" itself.
 * Example does not need to do anything for help_cmd if it doesn't want/need special output.
 */

// ------------------------------------------------------------------------------------------------
// Defines argument rules for "example2"
// ------------------------------------------------------------------------------------------------

static const ws_command_argument_t example2_args[] = {
	// Defines the "log" argument, which is a string, and can be invoked using '-l' or '--log'
	{
		.type = WS_ARG_TYPE_STRING,
		.required = false,
		.name = "log",
		.shortform = "l",
		.description = "Logs the provided argument string."
	}
};

// Example 2 can be invoked with "example2", "ex2", "exam2"
const char* const example2_aliases[] = { "ex2", "exam2" };
ws_command_t cmd_example2 = { 0 };

int example2(int argc, char** argv) {
	ws_context_t* ctx = ws_getCurrentContext();

	if (!ws_parse_args(ctx, argc, argv)) {
		return -1;
	}

	// Check if the "-l" / "--log" argument was passed
	if (ws_has_arg(ctx, "log")) {
		const char* log_val = ws_get_string(ctx, "log");
		ws_logger(WS_INFO, "%s", log_val);
	} else {
		ws_setForegroundColor(WS_FG_BRIGHT_CYAN);
		printf("This command requires a flag. Type \"help example2\" to learn more.\n");
		ws_setConsoleColors(ws_getDefaultColors());
	}

	return 0;
}

/* This will "override" the default output from WallShell.
 * Instead of printing the normal help message, it will call this function.
 * You can output anything you want here, it is treated like a normal command.
 *
 */
void example2_help(int argc, char** argv) {
	(void) argc;
	(void) argv;

	ws_logger(WS_INFO, "This is a custom help command.\n");

	// Built-in help printer automatically formats description based on registered args
	ws_printCommandHelp(&cmd_example2);
}

int main() {
#ifdef THREADED_SUPPORT
	// This is a single threaded application, we dont need the threadID.
	ws_doPrintThreadID(false);
#endif

	// Changes the prefix from "> " to "$ "
	ws_setConsolePrefix("$ ");

	// Sets console locale (UTF-8 on Windows, no-op on POSIX)
	ws_setConsoleLocale();

	// Register "example" command using explicit field assignments
	// This is done here, rather than in a global space like example2, because we don't really need it outside this function.
	ws_command_t cmd_example = {
		.command_name = "example",
		.main_func = example,
		.arguments = example_args,
		.arguments_count = sizeof(example_args) / sizeof(example_args[0])
	};
	ws_registerCommand(cmd_example);

	// Set up our globally defined example 2

	cmd_example2.command_name = "example2";
	cmd_example2.aliases = example2_aliases;
	cmd_example2.alias_count = 2;
	cmd_example2.main_func = example2;
	cmd_example2.help_func = example2_help;
	cmd_example2.arguments = example2_args;
	cmd_example2.arguments_count = sizeof(example2_args) / sizeof(example2_args[0]);

	ws_registerCommand(cmd_example2);

	// Main terminal function. Always call this last.
	ws_terminalMain();

	// Always make sure to clean up the terminal.
	// Always call this before exiting.
	ws_cleanAll();

	return 0;
}