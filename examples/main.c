/**
 * @file main.c
 * @author MalTheLegend104
 * @brief Example 1
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