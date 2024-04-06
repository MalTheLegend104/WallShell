# WallShell Logger

WallShell, by default, provides a very simple implementation of a logger.

## Logging

Every logger function requires a logtype. This is defined in the following enum:

```c
typedef enum {
	WS_LOG,
	WS_DEBUG,
	WS_INFO,
	WS_WARN,
	WS_ERROR,
	WS_FATAL
} ws_logtype_t;
```

There are two main logging functions:

### `void ws_logger(ws_logtype_t type, const char* format, ...)`

This function acts like a normal `printf` function.
It takes the same type of `printf` format string, same args, etc.

### `void ws_vlogger(ws_logtype_t type, const char* format, va_list args)`

This function acts exactly the same as `vprintf`

> Both logging function print a newline character after your format string.

## Changing Colors

If you so desire, you can change logger colors:

```c
void ws_setLoggerColors(ws_logtype_t type, ws_fg_color_t fg, ws_bg_color_t bg)
```

It changes the colors for the corresponding logtype.
It will remain this way until it is changed to something else.

These are the default colors for each log type:

```c
ws_color_t log_colors = {WS_FG_WHITE, WS_BG_DEFAULT};
ws_color_t debug_colors = {WS_FG_BRIGHT_GREEN, WS_BG_DEFAULT};
ws_color_t info_colors = {WS_FG_BRIGHT_CYAN, WS_BG_DEFAULT};
ws_color_t warn_colors = {WS_FG_BRIGHT_YELLOW, WS_BG_DEFAULT};
ws_color_t error_colors = {WS_FG_BRIGHT_RED, WS_BG_DEFAULT};
ws_color_t fatal_colors = {WS_FG_RED, WS_BG_DEFAULT};
```

## Thread IDs

If you have threaded support enabled, logging functions will print out the thread ID of the calling thread.
You can disable this by calling `ws_doPrintThreadID(false)`.

You can also add names to the threads. Most thread IDs are simply their handles (so usually a random int).
You can add a name for the calling thread by doing `ws_setThreadName("name")`,
and remove a thread name by doing `ws_removeThreadName(name)`.

> Removing a thread name can be done from any thread, setting a thread name must be done from the thread you are naming.
> Setting a thread name does use memory, it is advised to remove it whenever a thread closes.