# Getting Started

Setting up WallShell is straightforward. Follow these steps:

1. Place `wall_shell.c`, `wall_shell.h`, and `wallshell_config.h` in the same directory within your project.
2. Add `wall_shell.c` to your build system to ensure it gets compiled.
3. Include `wall_shell.h` and `wallshell_config.h` in your project's include paths.
4. Configure WallShell by adjusting options either in your build system or `wallshell_config.h`. Refer to the [configuration options](options.md) for more details.

Voilà! You're ready to use WallShell in your project.

## Where Next?

- Check out [configuration options](options.md) to see how to customize WallShell to your requirements.
- Look at the [in-depth example](../main.c) to see how WallShell can be integrated and used in practice.
- It's highly advised for everyone to read [things to note](things_to_note.md) after the above two.
- If you're implementing WallShell in a freestanding environment, ensure you understand the [standard library requirements](standard.md).
