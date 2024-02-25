PREVIOUS_BUF_SIZE

MAX_COMMAND_BUF

THREADED_SUPPORT

DISABLE_MALLOC

CUSTOM_CURSOR_CONTROL

CLEAR_ROW

- Defaults to virtual terminal sequences.
- Expected to clear the row the cursor is currently in.
- Does not have to change the cursor position, but it can reset it to the first column.

## Commands

- `NO_BASIC_COMMANDS`
    - Defining this will result in NO commands being built in.
        - clear
        - exit
        - help
        - history
    - It is highly recommended that you dont disable these, unless you need to.

You can disable individual commands one at a time.

- `NO_CLEAR_COMMAND` - disables clear command
- `NO_EXIT_COMMAND`- disables exit command
- `NO_HELP_COMMAND` - disables help command
- `NO_HISTORY_COMMAND` - disables history command

> It is advised you do not disable help or exit.
> You have no way of making your own (unless you modify the source code).
> Both work on `DISABLE_MALLOC` and `THREADED_SUPPORT`.

CUSTOM_CONSOLE_SETUP