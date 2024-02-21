#include "wall_shell.h"

int main() {
	setConsolePrefix("> ");
	setConsoleLocale();
	console_color_t colors = { CONSOLE_FG_MAGENTA, CONSOLE_BG_DEFAULT };
	setConsoleDefaults(colors);
	terminalMain();
}