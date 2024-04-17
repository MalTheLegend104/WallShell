# Things to Note

This file contains a list of things that are important to take note of when using WallShell.

## C99

The entire core of WallShell is written with strict ISO C99 adherence in mind. It uses your systems implementation of threads and console control.

## Note on Streams

- `stdin` is *required* for input on windows and POSIX.
  - The ability to change `stdin` is mostly meant for freestanding environments.
  - You have to define `CUSTOM_CONSOLE_SETUP` and everything that it includes.
    - You *can* do this on Windows and POSIX, but you will have to deal with console setup yourself.
      WallShell expects *mostly* raw terminal input, look through the source to see what this implies.
- `stdout` is used as default output, you can redirect it.
  - You can also redirect error output, default `stderr`, but currently it's not being used.
- Multiple things writing/reading from the same streams.
  - If multiple things (other than or including WallShell) are writing to the same stream, things may be out of order
    or out of place.
    This *should not* affect thread-safety, although if you are reading from the same stream you're writing to, make
    sure to sanitize input.

### "Counts" are very important

- For any field in a struct that is a "count", like `aliases_count`, are expected to behave like `strlen`. It's the
  total count, not the indexes.
- WallShell will attempt to access any part of the array as long as our "count" says that it's there. If count is wrong,
  it's undefined behavior (although it'll probably cause a segfault of some type).

### Using WallShell in C++

WallShell is written in ISO C, so it's compatible without modifications. A few things need to noted though:

- Commands are expected to be statically defined.
  - It's possible to use class methods as commands, but for C to be happy with them, you have to statically define
    them.
  - If this isn't possible, create a function outside the class specifically for WallShell.
- All functions passed to WallShell ***MUST*** be marked `extern "C"`.
  - This ***does not*** mean that those functions cannot contain C++ code.
    `extern "C"` simply disables [name mangling](https://en.wikipedia.org/wiki/Name_mangling) for that function.
- Be careful with strings.
  - WallShell, being written in C, of course wants null terminated strings of `char*` type.
  - If using `std::string`, utilize the `c_str()` function when needed.
