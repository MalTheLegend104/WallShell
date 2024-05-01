# @file CMakeLists.txt
# @author MalTheLegend104
# @brief CMake file for all examples.
# @version v1.0
# @copyright
# Copyright 2024 MalTheLegend104
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)
project(WallShell C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# I would turn extensions off, but then things used to configure POSIX and windows systems wouldnt work.
# With extensions off, if you define all the CUSTOM_* defines, it is fully compliant with C99
# set(CMAKE_C_EXTENSIONS OFF)

# Configure WallShell by doing this or using wallshell_config.h
# This also works for other build systems, like meson or make, or even just passing directly to your compiler.
# All you need is "-D<config>", which is supported by msvc, gcc, and clang, along with almost all other c compilers.
# add_definitions(-DNO_BASIC_COMMANDS)
add_definitions(-DTHREADED_SUPPORT)

add_executable(example_main_exe
	../wall_shell.h
	../wall_shell.c
	../wallshell_config.h
	main.c
)

add_executable(threaded_exe
	../wall_shell.h
	../wall_shell.c
	../wallshell_config.h
	threaded_example.c
)

# You can uncomment this to build the freestanding example.
# Please read the comment near the top of freestanding.c before doing so.
# add_executable(freestanding_exe
# freestanding/wall_shell.h
# freestanding/wall_shell.c
# freestanding/wallshell_config.h
# freestanding/wallshell_config.c
# freestanding/freestanding.c
# )

# For simplicity's sake, we define all our targets in one makefile.
# To build a certain one, first run cmake configure "cmake -B build ."
# Then run "cmake --build build --target <target>"
# This is a list of all possible targets:
# example_main -> The main example showing the most general implementation
# threaded     -> Shows how to configure and run WallShell in a multithreaded application.
# freestanding -> Shows a "freestanding" implementation of WallShell.

# General Example
add_custom_target(example_main
	COMMAND $<TARGET_FILE:example_main_exe>
	COMMENT "Running main example."
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	VERBATIM
)

# Threaded example
add_custom_target(threaded
	COMMAND $<TARGET_FILE:threaded_exe>
	COMMENT "Running threaded example."
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	VERBATIM
)

# You can uncomment this to build the freestanding example.
# Please read the comment near the top of freestanding.c before doing so.
# add_custom_target(freestanding
# COMMAND $<TARGET_FILE:freestanding_exe>
# COMMENT "Running threaded example."
# WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
# VERBATIM
# )
