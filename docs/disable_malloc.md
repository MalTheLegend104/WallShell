# Disable Malloc

This file lists a few nuances related to the `DISABLE_MALLOC` flag.

### Memory usage implications

Just because you disable malloc usage, *does not* mean that the application will use less memory.
In fact, it's often the opposite.

When you disable malloc, everything has to be statically defined.
This includes things like the list that stores commands, command buffer size, etc.
Often these get allocated more memory than they will use, something that does not happen when malloc is enabled.

### Almost all char** MUST be declared outside of context.

You cannot declare *any* `char**` in context.
It will get destroyed when the function declaring them exits, causing a memory access violation when trying to access
them.
If you declare them ***outside*** the scope of a function, it will behave as expected.
> The exception to this is `help` commands. If you create `HelpEntry`'s inside the same scope that you print them,
> they will behave properly. This mostly applies to aliases.

### Threaded support is not allowed when `DISABLE_MALLOC` is defined.

Typical threaded systems have at least some sort of memory management.
Threaded support requires the ability to allocate a few structures, namely mutex's and atomic bools.

## Example of a perfect setup

This command can be called using `example`, `ex`, or `exam`, and `help example` (or any of its aliases), will display
the help message. Running `help example a` will display the specific help message for `example a`.

```c
const char* example_aliases[] = { "ex", "exam" };
int example_command(int argc, char** argv) {
	// Print argc
	printf("argc: %d\n", argc);

	// Print argc
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}

	return 0; // Success
}

int example_help(int argc, char** argv) {
	if (argc > 1) {
		// Specific Help
		for (int i = 1; i <= argc; i++) {
			// 
			if (strcmp(argv[i], "a") == 0) {
				const char* required[] = {
					"-a     -> desription of the flag -a",
					"-asdf  -> desription of the flag -asdf"
				};
				const char* optional[] = {
					"-d     -> desription of the flag -d",
				};
				HelpEntry entry = {
					"Example A",
					"Description of the command.",
					required,
					2,
					optional,
					1
				};
				ws_printSpecificHelp(&entry);
			}
			// Check for other specific commands as you wish
		}
	} else {
		// General Help
		const char* commands[] = {
			"a      -> <description of command a>",
			"asdf   -> <description of command a>"
		};

		HelpEntryGeneral entry = {
			"Example",
			"Example description.",
			commands,
			2,
			example_aliases,
			2
		};

		ws_printGeneralHelp(&entry);
	}

	return 0;
}

// Somewhere else in the same file 
// OR 
// Somewhere that can see example_aliases & both function declarations
int main() {
	// Initialization code
	ws_registerCommand((ws_command_t) {example_command, example_help, "example", example_aliases, 2});
	// Register other commands
	ws_terminalMain();
}
```