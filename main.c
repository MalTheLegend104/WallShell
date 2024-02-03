#include "wall_shell.h"

int main() {
	setConsoleLocale();
	console_color_t colors = {CONSOLE_FG_MAGENTA, CONSOLE_BG_DEFAULT};
	printf("%d\n", setConsoleDefaults(colors));
	printf("%d - %d\n", getDefaultColors().foreground, getDefaultColors().background);
	terminalMain();
}