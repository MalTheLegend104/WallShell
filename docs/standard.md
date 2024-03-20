# Standard Library Usage

A big goal of WallShell is to use as little of the standard library (libc) as possible,
while still providing a feature-rich command line interface.

- All standard library function calls are expected to work as specified in the ISO C99 standard.
    - [glibc](https://www.gnu.org/software/libc/),
      [musl](https://musl.libc.org/),
      [mlibc](https://github.com/managarm/mlibc), and
      [msvc's libc](https://learn.microsoft.com/en-us/cpp/c-language/c-language-reference?view=msvc-170)
      are guaranteed to work.

### Note on MSVC

MSVC (the Microsoft C Compiler) "deprecates" many of the libc functions used, stating that they are not thread-safe.
This is irrelevent in WallShell. The contexts in which these functions are used ensure thread-safety.
WallShell disables warning `4996` (which tells you the functions are deprecated and prevents you from compiling), only
for the `wall_shell.c` source file.

> **NOTE:** The rest of this file is meant for freestanding environments.
> If you are not compiling for a freestanding environment, or rolling your own libc, ignore the rest of this file.

All functions and expected behavior can be found in at [cppreference](https://en.cppreference.com/w/c/header).
Please note that WallShell expects the C99 versions of these functions.
Listed below are the required headers & functions that are used from them.

## Freestanding Headers

These headers *should* be provided by your compiler, and don't need to be manually created or added by your standard
library:

- `stdint.h`
- `stdbool.h`
- `stddef.h`
- `stdarg.h`

## Libc Headers

These headers/functions typically have to be defined by your standard library (or you) themselves.

### `string.h`

- `strtok`
    - This is only needed if `DISABLE_MALLOC` is not defined. It typically uses `malloc()` internally.
- `strlen`
- `strcmp`
- `strcpy`
- `memcpy`
- `memset`

### `stdlib.h`

#### If `DISABLE_MALLOC` is not defined:

- `malloc`
- `realloc`

#### If `DISABLE_MALLOC` is defined:

- Nothing (the header is still expected to exist, even if empty).

### `stdio.h`

- `fprintf`
    - This is probably the biggest problem for freestanding environments without a standard lib.
    - A simple solution is to map this to `printf`, and define the following:
        - `FILE`
        - `stdin`
        - `stdout`
        - `stderr`
        - These definitions don't necessarily have to do anything, they just need to be defined.
