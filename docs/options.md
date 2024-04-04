PREVIOUS_BUF_SIZE

- Determines how much history the console keeps track of.

MAX_COMMAND_BUF

- Determines the maximum size of the command buffer.
- Defaults to 256. If you expect your commands to have longer names, arguments, etc., increase this.
- Ideally, you should keep this as low as possible. Memory usage greatly increases the larger you make this.

NO_WALLSHELL_LOGGING

Disables the use of logging functions.

- All logging functions are wrappers around `vfprintf`.

THREADED_SUPPORT

> Important note: Threaded support ***requires*** malloc.
> You cannot use both `DISABLE_MALLOC` and `THREADED_SUPPORT` at the same time.

Adds support for a threaded implementation of WallShell.
This allows you to use it in its own thread, separate from your main program.

- Provides a wrapper around `mutex` for `pthread` and windows threads.
- Provides an implementation of an `atomic_bool`.

> If you are using WallShell in a freestanding environment, see the `CUSTOM_THREADS` flag.

CUSTOM_THREADS

If you are in a freestanding environment,
or you wish to use your own wrapper on an already supported system,
you must define the following functions:

```c
typedef /* Mutex type */ ws_mutex_t;
typedef /* ThreadID Type */ ws_thread_id_t;

void ws_lockMutex(ws_mutex_t* mut);
void ws_unlockMutex(ws_mutex_t* mut);
ws_mutex_t* ws_createMutex();
void ws_destroyMutex(ws_mutex_t* mut);

ws_thread_id_t getThreadID();
void ws_printThreadID(FILE* stream);

void ws_sleep(size_t ms);
```

> The function declarations are already provided for you.
> You do have to retype the entire typedef for `ws_mutex_t` and `ws_thread_id_t`.

These are further explanations:

- `ws_mutex_t` is simply expected to be whatever handle your threads have.
- `ws_thread_id_t` should be a unique identifier for each thread.
  - Since you're the one implementing `getThreadID()`, this can be anything.
  - Most systems, like Windows, Linux, and macOS all use `unsigned long`
- `void ws_lockMutex(ws_mutex_t* mut);`
  - Expected to lock the provided mutex. Mutex is always a pointer to mutex object.
- `void ws_unlockMutex(ws_mutex_t* mut);`
  - Expected to unlock the provided mutex. Mutex is always a pointer to the mutex object.
- `ws_mutex_t* ws_createMutex();`
  - Creates a mutex. Should return a pointer to a mutex, and `NULL` if one couldn't be created.
- `void ws_destroyMutex(ws_mutex_t* mut);`
  - Destroys the provided mutex. Mutex should be set to `NULL`, and ideally should be locked before destruction.
    - Locking it before destruction ensures that anything currently using it finishes first.
- `ws_thread_id_t getThreadID();`
  - Should return the thread ID for the calling thread.
  - The thread ID can be anything, it's just expected to be unique for each thread.
- `void ws_printThreadID(FILE* stream);`
  - Expected to print the calling threads thread ID to the stream.
  - This is required, since `fprintf` requires a format, and not all systems have the same thread identifiers.
- `void ws_sleep(size_t ms);`
  - Sleep the calling thread for `ms` milliseconds.

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
  - `void ws_moveCursor(ws_cursor_t direction);`
    - This function moves the cursor *once* in the specified direction.
  - `void ws_moveCursor_n(ws_cursor_t direction, size_t num);`
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

ws_get_char (non-blocking)
ws_get_char_blocking
PRINTING_NEEDS_FLUSH