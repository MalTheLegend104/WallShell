
#include "../wall_shell.h"
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

/**************************************************************************************************
 * This example simply launches WallShell in a different thread, prints a few things, then waits
 * for the user to be finished with the shell. There are a few lines you can uncomment to change
 * the behavior to demonstrate stopping the shell along with showing how to configure it.
 *************************************************************************************************/

int main() {
	// All configuration must happen *before* wallshell is running.
	// You can also stop wallshell, reconfigure it, and restart it.
	ws_setConsolePrefix("$ ");

#ifdef _WIN32
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ws_terminalMain, NULL, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, ws_terminalMain, NULL);
#endif
	// This is an example of what 
	ws_setThreadName("Main");
	for (int i = 0; i < 10; i++) {
		ws_logger(WS_LOG, "%d", i);
	}

	// If you want to stop WallShell for any reason, do the following:
	// ws_stopTerminal();
	// You should still join the thread and wait for it to finish.
	// Unless you stopped the terminal during a user prompt, it will exit nicely.
	// You should still run ws_cleanAll(), but it's not requried if your exiting the program entirely.

#ifdef _WIN32
	WaitForSingleObject(thread, INFINITE);
#else
	pthread_join(thread, NULL);
#endif

	ws_cleanAll();
}