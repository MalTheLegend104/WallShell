# Examples

This folder provides a few in depth examples. Descriptions of each can be found below, as well as how to build them.

## Building

A `CMakeLists.txt` is provided to allow for easy CMake builds, although you can also compile them manually.
If you wish to use CMake, follow these steps:

1. Run `cmake -B build .` in the current directory.
2. Run `cmake --build build --target <target>`.
   - `<target>` should be replaced with the name of the target as described in the [descriptions](#descriptions) section.

That's it. CMake will automatically run the example relating to the target you picked.

## Descriptions

### Main

> CMake Target: `example_main`

The main example shows you how to configure WallShell, as well as how to create and register a command.

### Threaded

> CMake Target: `threaded`

This example shows you how to set up WallShell in a threaded environment.

### Freestanding

> CMake Target: `freestanding`
> NOTE: This target is not included by default. Please read the CMakeLists.txt, as well as freestanding.c to understand why.

This example provides a skeleton to follow for implementing in a freestanding or baremetal environment.
The entire example is self contained in the `freestanding` directory.
Most of the example is in `wallshell_config.h`, but make sure to read through `freestanding.c` first.
