# Contribution Guide

Thank you for considering contributing to WallShell! Your contributions, no matter how small, are incredibly valuable to the project's growth and improvement.

## Table of Contents

- [Personal Forks](#personal-forks)
- [Code Styling](#code-styling)
- [Pull Request Denial](#pull-request-denial)

## Personal Forks

All code contributions should ideally be done in a personal fork or another branch.

1. Fork this repository.
2. Implement your changes or fixes in your fork.
3. Create a pull request to submit your changes back to the main repository.

## Code Styling

There are a few code styling requirements:

- WallShell uses tabs. Spaces are only to be used in markdown files.
- External Prefixes
  - All external (included in `wall_shell.h`) functions must be prefixed `ws_`.
- Internal Prefixes
  - All internal (only defined in wall_shell.c) functions must be prefixed `ws_internal`.
- Enum Members
  - All enum members, internal and external, must be prefixed with `WS_`.
  - Should be in `SCREAMING_SNAKE_CASE`.
- Struct Members
  - Must be in `lower_snake_case`.
  - Function pointers are an exception, they must be in `camelCase`.
- Functions
  - All parameters must be `camelCase`.
  - All function names, after the prefix, should be in `camelCase`.
  - Since C doesn't have function overloading, sometimes functions have to take several types.
    - These can be put on as a suffix in the form `prefix_functionName_suffix()` where the suffix describes the type.
    - This suffix should should be very few characters. Char could be `_c`, int could be `_i`, etc.
- Internal Variables
  - Case be whatever you want, but keep it consistent within a function.
  - Make the variable names descriptive but not incredibly long.
- Global Variables
  - All global variables should be in `lower_snake_case`.
- Typedefs
  - Must be prefixed with `ws_`.
    - This is the same for internal and external.
  - Must be suffixed with `_t`.
- Braces
  - [K&R](https://en.wikipedia.org/wiki/Indentation_style#One_True_Brace), particularly the "one true brace style" variant.
  - TLDR: Brace on the same line as the declaration, `} else {`, and things should be one indent apart.
  - See other examples in `wall_shell.h` and `wall_shell.c`.
- Doxygen Comments
  - All doxygen comments must be in JavaDoc form.
  - All functions, enums, structs, unions, and typedefs must be docuemented. This includes internal functions.
    - Mark internal functions with the `@internal` flag.
    - All fields/members must be documented.
  - All enums, structs, and unions that are defined in `wall_shell.h` must be commented at the *bottom* of `wall_shell.c`.
    - Follow the ones that are already there.
    - The exception is system dependent typedefs. Those can be commented where they are.
- No functions should be defined in `wall_shell.h`, only declared. All function definitons should be in `wall_shell.c`.
- All structs, unions, and enums should be `typedef`'d.
  - See `wall_shell.h` for examples.
- Commenting rules are pretty lax, just comment complex sections of code well enough that they can be understood at a glance.
  - Keep comment styles consistent within a function.
  - Seperate sections using the same style that already exists in `wall_shell.c`, using "bar" comment blocks.
- Defines
  - Almost all defines should be in `SCREAMING_SNAKE_CASE`, the same with macros.
  - The only exception is macros that take a parameter, they can be either `SCREAMING_SNAKE_CASE` or `lower_snake_case`.

## Pull Request Denial

- The repo owner reserves all rights to deny a pull request for any reason. This isn't meant to be intimidating; rather, it's to ensure that all contributions align with the project's goals and standards.
- If you have an idea for a feature, it's a good idea to create an issue and mention that you'd like to implement it. This way, you'll get feedback on it before you spend time implementing it.
- If your pull request is denied for code quality or code styling issues, simply fix them and create another pull request.

> If you feel you were improperly denied, **cite this document.** This document isn't perfect, there may be errors.
> The reviewer could also improperly interpret something or forget certain rules. Be polite.
>
> If it's decided that:
>
> 1. You were wrong.
>     - Fix it and resubmit your pull request.
> 2. The reviewer was wrong.
>     - As long as the rest of the pull request looked good, it will be accepted.
>     - If there were other issues, it will still be denied until they are fixed.
> 3. This document was wrong.
>     - This document will be properly updated.
>     - Your pull request will be looked at again.
