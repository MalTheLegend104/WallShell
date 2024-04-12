#include "../wall_shell.h"

int main() {
	// ws_setThreadName("Main");
	ws_doPrintThreadID(false);
	ws_logger(WS_LOG, "This is a log message.");
	ws_logger(WS_DEBUG, "This is a debug message.");
	ws_logger(WS_INFO, "This is a info message.");
	ws_logger(WS_WARN, "This is a warn message.");
	ws_logger(WS_ERROR, "This is a error message.");
	ws_logger(WS_FATAL, "This is a fatal message.");

	ws_setConsolePrefix("> ");
	ws_setConsoleLocale();
	ws_terminalMain();
}