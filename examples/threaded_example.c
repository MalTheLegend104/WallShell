/**
 * @file threaded_example.c
 * @author MalTheLegend104
 * @brief Threaded example
 *
 * This file shows you how to use threads in combination with WallShell.
 *
 * @version v1.0
 * @copyright
 * Copyright 2024 MalTheLegend104
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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