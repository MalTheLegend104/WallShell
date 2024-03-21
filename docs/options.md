PREVIOUS_BUF_SIZE

- Determines how much history the console keeps track of.

MAX_COMMAND_BUF

- Determines the maximum size of the command buffer.
- Defaults to 256. If you expect your commands to have longer names, arguments, etc., increase this.
- Ideally, you should keep this as low as possible. Memory usage greatly increases the larger you make this.

THREADED_SUPPORT

Not implemented yet. There isn't wide libc support (not in glibc nor msvc) for the C11 `threads.h`.

- A solution will likely come out later, with a wrapper around `pthreads` and Window's threads.
- As a result of this, any non-unix or windows based machines would have to implement their own thread wrapper.
    - This shouldn't be a large undertaking. WallShell only uses `mutex`'s and an `atomic_bool`.

CUSTOM_THREAD_WRAPPER

DISABLE_MALLOC

- Meant for freestanding environments that don't necessarily have access to normal memory allocation.
- Requires everything to be statically defined.
- You have to be very careful as to what context things are defined in.
    - Defining things like aliases in the scope of a function can lead to major problems.
    - [See this for more specifics.](disable_malloc.md)

There are two parameters as a result of disabling malloc:

- COMMAND_LIMIT
    - Sets the maximum amount of commands that can be added to the handler.
    - Defaults to 25
    - You should set it to be the amount of commands you are using.
        - If you need to dynamically add/remove commands during runtime, make sure you make this big enough to
          accommodate that.
    - If you try to add more commands after it's reached the maximum, it just ignores the new commands.
- MAX_ARGS
    - Max amount of arguments in `argv`. Normally there is limitless arguments in argv (as long as it fits within
      MAX_COMMAND_BUF).
    - Defaults to 32
        - 32 should be a reasonable maximum, but if you find yourself using more than 32 arguments, increase this.
        - This doesn't necessarily have a large impact on memory usage, but making it excessively large will.

CUSTOM_CURSOR_CONTROL

- Allows you to use different methods to control the console cursor than virtual sequences.
- You must create definitions for two functions:
    - `void wallshell_move_cursor(console_cursor_t direction);`
        - This function moves the cursor *once* in the specified direction.
    - `void wallshell_move_cursor_n(console_cursor_t direction, size_t num);`
        - This function moves the cursor `num` times in the specified direction.
  > It's advised that you put these definitions in `wallshell_config.h`.
  > Remember that these are already *defined* in `wall_shell.h`, redefining them will cause errors.

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

wallshell_get_char (non-blocking)
wallshell_get_char_blocking
PRINTING_NEEDS_FLUSH