# WallShell

Portable command handler using C99 (with optional C11 support for threads).

## Portability

### Freestanding Environments

A big focus of WallShell was to create a shell for freestanding environments.
With proper configuration, WallShell only requires a very minimal portion of the standard library.
This is great for embedded systems, operating systems, and any other low level application.

### Large Scale Projects

Customizations make WallShell applicable to large scale applications as well.
Some example use cases:

- Simple Debug Console for Development
    - Can easily run alongside a full scale GUI and used to configure application state, configurations, monitoring
      internal tasks, etc.
- Frontend for Backend Applications
    - Can act as a frontend interface for managing and monitoring the application's internal state, configurations, and
      performing administrative tasks, all without the burden of creating a full scale GUI.

The sky's the limit. You define the commands that are run and what they do. WallShell only needs a function pointer.

### C99 and C11

The entire core of WallShell is written with strict ISO C99 adherence in mind.
C11 is *only* required for threads.
`<thread.h>` and `<stdatomic.h>` are the only C11 features used, and are not used by default.

## Note on Streams

- `stdin` is *required* for input on windows and POSIX.
  - The ability to change `stdin` is mostly meant for freestanding environments.
  - You have to define `CUSTOM_CONSOLE_SETUP` and everything that it includes.
    - You *can* do this on Windows and POSIX, but you will have to deal with console setup yourself.
      WallShell expects *mostly* raw terminal input, look through the source to see what this implies.
- `stdout` is used as default output, you can redirect it.
  - You can also redirect error output, default `stderr`, but currently it's not being used.