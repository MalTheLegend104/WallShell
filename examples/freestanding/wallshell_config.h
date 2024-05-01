/**
 * @file wallshell_config.h
 * @author MalTheLegend104
 * @brief Freestanding Example
 *
 * This file is an example of how a freestanding environment may set up its config.
 * Unlike the normal wallshell_config.h, this file *does* have a license, since it's an example.
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
#ifndef WS_CONFIG_H
#define WS_CONFIG_H

// WallShell relies on many platform dependent functions and behaviors.
// In a freestanding environment, WallShell needs to know to handle those, and where to get them from.
// There are a few main categories that stem from these problems:
// Console Setup  -> Every console is set up differently. Some consoles may need minimal configuration,
//                   while others you have to configure everything to get it to a state that WallShell needs.
//                   WallShell also depends on a few system dependent input functions.
// Console Colors -> Similar to console setup, each console supports different color controls. 
//                   Most terminal support virtual terminal sequences, which is what WallShell defaults to. 
//                   A freestanding environment is unlikely to support virtual terminal sequences, therefore  
//                   you must help WallShell set the console colors. If no colors are supported at all, it   
//                   still needs to think that they are.
// Cursor Control -> Very similar to console colors, cursor control relies on virtual sequences by default.
//                   If the environment doesnt support them, you must help WallShell by telling it how to
//                   move the cursor around.
// Threads        -> The C standard doesn't do much help in terms of threaded support. There is thread.h but
//                   it's widely unsupported, and it's generally easier to use system implementations.  
//                   WallShell provides a very small wrapper around mutex's and defines it's own atomic bools. 
//                   If your freestanding environment supports threads, you have to give WallShell some  
//                   information about them so it can properly use them. If THREADED_SUPPORT is not enabled,
//                   this point is entirely irrelevant.
// As a result, this example will be broken down by each category, to show you how to deal with each one seperately.
//
// IMPORTANT NOTE: It's highly advised that you *DO NOT* define these functions in this config file.
// Almost all of them rely on things defined in wall_shell.h, and will create a circular include.
// If you define these functions in a seperate file, it's way easier to deal with this problem.
// In contrast, the macros *MUST* be defined in this file.

/* You can comment out the defines to turn off sections of the example. */

/* Console Setup */
#define CUSTOM_WS_SETUP

#ifdef CUSTOM_WS_SETUP
// For the sake of this example, let's say our enviroment doesn't have any special locale to set.
// We can just leave the locale define blank.
#define SET_TERMINAL_LOCALE
/* We also need two special types of input. Blocking and Non-blocking getc()
 * In our pretend environment here, lets say our blocking function is simply "fgetc()",
 * and our non-blocking function is "nb_getchar()". WallShell needs to know where they are defined.
 * For this example. We'll create the function declarations here and define them in wallshell_config.c
 * but on a real system this would likely require you to include a header here of some kind. 
 */
int nb_getchar();
#define ws_get_char(stream) fgetc(stream)
#define ws_get_char_blocking(stream) nb_getchar()
/* Please keep in mind that the non-blocking function must return -2 if there is nothing in the input stream.
 * Also note that the defines have streams. These are simple FILE*, and dont have to be used.
 * They simply exist in case your getchar functions are similar to fgetc() 
 */

// While not directly related to CUSTOM_WS_SETUP, there exists a macro called CLEAR_ROW that is used to clear a row.
// Normally this is done using virtual sequences, but if you dont have virtual sequences you'd do the following:
// #define CLEAR_ROW <function to clear a row>
// Read more about it in the docs to see what's actually required by this.
#endif // CUSTOM_WS_SETUP


/* Console Colors */
#define CUSTOM_WS_COLORS

#ifdef CUSTOM_WS_COLORS
/* Custom colors are fairly simple to do.
 * You only need two defines, but you will likely have to implement your own functions.
 * Currently WallShell doesn't allow you to include the enums relating to colors in this file.
 * The linker should hopefully resolve the function, but you may need to declare it here using ints instead.
 * For the purpose of this example, we'll just define the function "set_color()" and pretend it 
 * sets the consoles colors to those provided.
 */
#define SET_WS_COLORS(a, b) set_color(a, b);

// WallShell needs to know how to reset the colors to their console defaults.
void reset_console();
#define RESET_CONSOLE reset_console()
#endif 

/* Cursor Control */
#define CUSTOM_CURSOR_CONTROL

#ifdef CUSTOM_CURSOR_CONTROL
/* As stated above, the cursor control typically uses virtual sequences. 
 * If you dont have support for virtual sequences, you have to define two functions.
 * Since the declarations are already defined in wall_shell.h, nothing special needs to be done in this header.
 * The functions will be defined in wallshell_config.h
 */
#endif 

/* Custom Threads */
#define THREADED_SUPPORT

#ifdef THREADED_SUPPORT
#define CUSTOM_THREADS 

#ifdef CUSTOM_THREADS
// The only things that have to be defined in this header are the two typedefs. 
// For our example, let's assume our environment has an implementation of pthreads.
#include <pthread.h>
#include <stdint.h>
typedef /* Mutex type */ ws_mutex_t;
typedef uint64_t ws_thread_id_t;
#endif // CUSTOM_THREADS
#endif // THREADED_SUPPORT

// All of the actual function defintions are in wallshell_config.c
// wall_shell.h only needs to know these defines. The linker will resolve the locations later.

#endif // WS_CONFIG_H