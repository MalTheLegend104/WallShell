PREVIOUS_BUF_SIZE

- Determines how much history the console keeps track of.

MAX_COMMAND_BUF

- Determines the maximum size of the command buffer.
- Defaults to 256. If you expect your commands to have longer names, arguments, etc., increase this.
- Ideally, you should keep this as low as possible. Memory usage greatly increases the larger you make this.

THREADED_SUPPORT

DISABLE_MALLOC

- Meant for freestanding environments that don't necessarily have access to normal memory allocation.
- Requires everything to be statically defined.
- You have to be very careful as to what context things are defined in.

CUSTOM_CURSOR_CONTROL

- Allows you to use different methods to control the console cursor than virtual sequences.
- You must define the `MOVE_CURSOR(direction)` macro, that takes in a `console_cursor_t`.

CLEAR_ROW

- Defaults to virtual terminal sequences.
- Expected to clear the row the cursor is currently in.
- It does not have to change the cursor position, but it can reset it to the first column.

## Commands

- `NO_BASIC_COMMANDS`
    - Defining this will result in NO commands being built in.
        - clear
        - exit
        - help
        - history
    - It is highly recommended that you do not disable these unless you need to.

You can disable individual commands one at a time.

- `NO_CLEAR_COMMAND` - disables clear command
- `NO_EXIT_COMMAND`- disables exit command
- `NO_HELP_COMMAND` - disables help command
- `NO_HISTORY_COMMAND` - disables history command

> It is advised you do not disable help or exit.
> You have no way of making your own (unless you modify the source code).
> Both work on `DISABLE_MALLOC` and `THREADED_SUPPORT`.

CUSTOM_CONSOLE_SETUP