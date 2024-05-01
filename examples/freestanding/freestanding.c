/**
 * @file freestanding.c
 * @author MalTheLegend104
 * @brief Freestanding Example
 *
 * This file is an example of how to implement a freestanding version of WallShell.
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
#include "wall_shell.h"

/**************************************************************************************************
 * THIS EXAMPLE WILL NOT BUILD.
 * In order for this to build & run, this folder would have to contain it's own versions of 
 * wall_shell.h and wall_shell.c. For the purposes of keeping this repository clean, 
 * these are not included in this examples directory. If you desire to build this to test it, 
 * simply copy wall_shell.h and wall_shell.c in this directory, uncomment the relevant part of 
 * the CMakeLists.txt, and then compile it as normal.
 *************************************************************************************************/

// For the sake of the example, this is a bare minimum main function. 
// To see options, look at other examples. 
// This example is mostly focused on the other files in this directory.
int main() {
	ws_terminalMain();
	ws_cleanAll();
}