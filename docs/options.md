# Compile-Time Options

This document contains a list of all compile time options available to WallShell.

## Table of Contents

- [Compile-Time Options](#compile-time-options)
  - [Table of Contents](#table-of-contents)
  - [Usage](#usage)
  - [PREVIOUS\_BUF\_SIZE](#previous_buf_size)
  - [MAX\_COMMAND\_BUF](#max_command_buf)
  - [NO\_LOGGING](#no_logging)
  - [THREADED\_SUPPORT](#threaded_support)
  - [DISABLE\_MALLOC](#disable_malloc)
    - [COMMAND\_LIMIT](#command_limit)
    - [MAX\_ARGS](#max_args)
  - [CUSTOM\_CURSOR\_CONTROL](#custom_cursor_control)
  - [CLEAR\_ROW](#clear_row)
  - [Commands](#commands)
  - [CUSTOM\_WS\_SETUP](#custom_ws_setup)
  - [CUSTOM\_WS\_COLORS](#custom_ws_colors)

## Usage

You can either use these options by defining them in `wallshell_config.h`,
or passing the defines on the command line using `-D<option>`.

If the option requires you to create your own function definitions or macros:

- You should put the definition or the include containing the definition in `wallshell_config.h`.
- Macro definitions should be exclusively in `wallshell_config.h`.

## PREVIOUS_BUF_SIZE

- Determines how much history the console keeps track of.
- Expected to be an integer, normally defaults to 50.

## MAX_COMMAND_BUF

- Determines the maximum size of the command buffer.
- Defaults to 256. If you expect your commands to have longer names, arguments, etc., increase this.
- Ideally, you should keep this as low as possible. Memory usage greatly increases the larger you make this.

> Future updates may allow the command buffer to be of any size,
> but keeping it a consistent size makes the program a lot simpler, and much easier to maintain.

## NO_LOGGING

Disables the use of logging functions.

- All logging functions are wrappers around `vfprintf`.

To read more about logging functions, see [this page](logging.md).

## THREADED_SUPPORT

> Important note: Threaded support ***requires*** malloc.
> You cannot use both `DISABLE_MALLOC` and `THREADED_SUPPORT` at the same time.

Adds support for a threaded implementation of WallShell.
This allows you to use it in its own thread, separate from your main program.

- Provides a wrapper around `mutex` for `pthread` and windows threads.
- Provides an implementation of an `atomic_bool`.

If you wish to implement your own thread wrapper,
or are in a freestanding environment that isn't supported out of the box,
see [this page](custom_threads.md).

## DISABLE_MALLOC

- Meant for freestanding environments that don't necessarily have access to normal memory allocation.
- Requires everything to be statically defined.
- You have to be very careful as to what context things are defined in.
  - Defining things like aliases in the scope of a function can lead to major problems.
  - [See this for more specifics.](disable_malloc.md)

There are two parameters as a result of disabling malloc:

### COMMAND_LIMIT

- Sets the maximum amount of commands that can be added to the handler.
- Defaults to 25
- You should set it to be the amount of commands you are using.
  - If you need to dynamically add/remove commands during runtime, make sure you make this big enough to
    accommodate that.
- If you try to add more commands after it's reached the maximum,
  it just ignores the new commands and keeps the old ones as are.

### MAX_ARGS

- Max amount of arguments in `argv`. Normally there is limitless arguments in argv (as long as it fits within
  MAX_COMMAND_BUF).
- Defaults to 32
  - 32 should be a reasonable maximum, but if you find yourself using more than 32 arguments, increase this.
  - This doesn't necessarily have a large impact on memory usage, but making it excessively large(50+) will.

## CUSTOM_CURSOR_CONTROL

- Allows you to use different methods to control the console cursor than virtual sequences.
- You must create definitions for two functions:
  - `void ws_moveCursor(ws_cursor_t direction);`
    - This function moves the cursor *once* in the specified direction.
  - `void ws_moveCursor_n(ws_cursor_t direction, size_t num);`
    - This function moves the cursor `num` times in the specified direction.
  > It's advised that you put these definitions in `wallshell_config.h`.
  > Remember that these are already *declared* in `wall_shell.h`, redeclaring them will cause errors.

## CLEAR_ROW

> Unlike most things on this list, this is a macro, not just simply a define.
> It's normally defined as: `#define CLEAR_ROW fprintf(ws_out_stream, "\033[M");`

- Defaults to virtual terminal sequences.
- Expected to clear the row the cursor is currently in.
- It does not have to change the cursor position, but it can reset it to the first column.
  - WallShell uses `\r` to return to the beginning of the line regardless.

## Commands

WallShell comes with a very minimal set of built-in commands:

- clear
- exit
- help
- history

These are mostly included to simplify implementation, as these require accessing internal variables and functions.
Despite this, you may want to disable either all or some of them:

- `NO_BASIC_COMMANDS`
  - Defining this will result in NO commands being built in.
  - It is highly recommended that you do not disable these unless you need to.

You can disable individual commands one at a time.

- `NO_CLEAR_COMMAND` - disables clear command
- `NO_EXIT_COMMAND`- disables exit command
- `NO_HELP_COMMAND` - disables help command
- `NO_HISTORY_COMMAND` - disables history command

> It is advised you do not disable help or exit.
> You have no way of making your own (unless you modify the source code).
> Both work on `DISABLE_MALLOC` and `THREADED_SUPPORT`.

## CUSTOM_WS_SETUP

On most pre-existing systems, you really want to avoid defining this.
This exists purely for either non-mainstream systems (not windows or unix based), or freestanding environments.
If you really need this, see [this page](custom_console_setup.md).

## CUSTOM_WS_COLORS

If your system uses something other than virtual sequences for console color,
you have to define this and implement the color changing function yourself.

There is only two things you need to define:

- `RESET_CONSOLE`
  - Macro that is expected to reset the color of the console to the console's default.
  - Normally defined as:

    ```c
    #define RESET_CONSOLE fprintf(ws_out_stream, "\033[0m")
    ```

- `SET_WS_COLORS(a, b)`
  - `a` is the `ws_fg_color_t`, `b` is the `ws_bg_color_t`
  - Ideally, you should define this to expand to a function:
    ```c
    void ws_internal_changeConsoleColor(ws_fg_color_t fg, ws_bg_color_t bg);
    #define SET_WS_COLORS(a, b) ws_internal_changeConsoleColor(a, b);
    ```
