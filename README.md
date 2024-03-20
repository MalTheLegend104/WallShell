# WallShell

Portable command handler using C99 (with optional C11 support for threads).

## Portability

### Freestanding Environments

A big focus of WallShell was to create a shell for freestanding environments.
With proper configuration, WallShell only requires a very minimal portion of the standard library.
This is great for embedded systems, hobby operating systems, and any other low level application.

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
