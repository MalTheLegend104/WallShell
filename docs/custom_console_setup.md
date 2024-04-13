# Custom Setup

You should only consider doing this if you are on a non-standard system that isn't Windows or Unix based.

There are several things that need to be defined:

## `ws_error_t setConsoleMode()`

- Expected to set the console mode to what WallShell expects.
  - WallShell expects the following conditions:
    - Echo for key presses is turned off.
      - When the user types, it should not be displayed. WallShell will display it if needed.
    - If your terminal supports them, enable virtual terminal sequence input and output.
    - Raw Buffering
      - The terminal should not wait for the user to press enter to send key presses to WallShell.

## `void ws_resetConsoleState()`

- Expected to reset the console state on systems where it's needed.
- Some systems, namely unix ones using `terminos`, don't go back to a default state when a program exits.
- WallShell will mess up your terminal if you don't properly reset it, as most programs don't need raw buffering or
  disable input echos.

## SET_TERMINAL_LOCALE

- Some terminals, namely all on windows, don't have UTF support in general.
- If you need support for a certain locale, you have to define this.
- This is only called when a user calls `ws_setConsoleLocale()`, so if you don't call this, you can leave the define
  blank.
  - It does still need to be defined, simply `#define SET_TERMINAL_LOCALE` is enough.

## Input

There are two macros that need to be defined:

### ws_get_char(stream)

WallShell expects to be able to "poll" for keyboard events, rather than waiting for `getc()`.
Normal `getc()` prevents the thread (or application) from being closed when asked.

The function that this expands to is expected to return an int (like `getc()` does) if available or `-2` otherwise.
> The -2 comes from the fact that `EOF` is defined as -1, and `getc()` can return `EOF` in certain circumstances.

- `stream` does not have to be used. It's provided simply for if you need to use something similar to `fgetc(stream)`.

### ws_get_char_blocking(stream)

In contrast to the other one, this one is expected to be a blocking function.
When prompting users, or getting virtual terminal sequences, blocking functions are much nicer to work with.
Same as above, `stream` does not have to be used.

> You can potentially define this to use `getc()`, but it's currently defined to use `getchar()` on unix and `_getch()`
> on windows.

## PRINTING_NEEDS_FLUSH

Sometimes, especially on unix systems, you have to flush stdout (or whatever stream your using), before fprintf (or
normal printf) will actually write to the buffer.
If your system requires the stream to be flushed, simply define this.
