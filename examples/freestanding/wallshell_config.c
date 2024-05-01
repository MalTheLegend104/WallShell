/**
 * @file wallshell_config.c
 * @author MalTheLegend104
 * @brief Example of a freestanding wallshell config.
 *
 * This file provides a skeleton to follow when implementing WallShell in a freestanding environment.~
 *
 * @version v1.0.0
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
#include "wallshell_config.h"
#include "wall_shell.h"

/**************************************************************************************************
 * QUICK DISCLAIMER: This file is meant to have very little in it. Most of this is just simply 
 * empty functions. Remember, this is not meant to be a working example. To see what each function
 * does, it is highly recommended to look at the Doxygen docs for each function.
 *************************************************************************************************/

/* Console Setup */
#ifdef CUSTOM_WS_SETUP
ws_error_t setConsoleMode(){ return WS_NO_ERROR; }
void ws_resetConsoleState(){ }
int nb_getchar(){
	// Make sure to check the input buffer.
	// If nothing is there for WallShell, return -2.
	return -2;
}
#endif

/* Console Colors */
#ifdef CUSTOM_WS_COLORS
void set_color(ws_fg_color_t fg, ws_fg_color_t bg){
	// You can use the enums to help you determine what color should be what.
	// The enum values are aligned to their virtual sequence colors.
	// You may have to create your own mappings in your environment if they are different.
}

void reset_console() {
	// This should reset the console colors to their defaults.
}
#endif

/* Cursor Control */
#ifdef CUSTOM_CURSOR_CONTROL
void ws_moveCursor(ws_cursor_t direction){
	// This is expected to move the cursor once in the provided direcction.
}

void ws_moveCursor_n(ws_cursor_t direction, size_t num){
	// This is expected to move the cursor num times in the provided direcction.
}
#endif

/* Threaded support */
#if defined(THREADED_SUPPORT) && defined(CUSTOM_THREADS)

void ws_lockMutex(ws_mutex_t* mut) {}
void ws_unlockMutex(ws_mutex_t* mut){}
ws_mutex_t* ws_createMutex() {
	// make sure to properly create a mutex.
	// Ensure it's a pointer, not a reference.
	return NULL;
}
void ws_destroyMutex(ws_mutex_t* mut){
	// It's advised to lock it before freeing it.
	// Make sure to set it to NULL
}
 
ws_thread_id_t getThreadID() {
	// This should return the threadID of the calling thread.
	return 0;
}
void ws_printThreadID(FILE* stream){
	// This should print the calling threads threadID to the provided stream.
}
 
void ws_sleep(size_t ms) {
	// This should sleep the calling thread for the provided amount of milliseconds.
}
#endif