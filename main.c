#include "wall_shell.h"

int main(){
	setConsoleLocale();
	console_color_t colors = { CONSOLE_FG_RED, CONSOLE_BG_BLUE };
	printf("%d\n", setConsoleDefaults(colors));
	printf("%d - %d\n", getDefaultColors().foreground, getDefaultColors().background);
	terminalMain();
}