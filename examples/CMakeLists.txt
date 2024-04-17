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

# set(CMAKE_C_EXTENSIONS OFF)

# configure doing this or using wallshell_config.h
# add_definitions(-DNO_BASIC_COMMANDS)
add_definitions(-DTHREADED_SUPPORT)

add_executable(WallShell
	../wall_shell.h
	../wall_shell.c
	../wallshell_config.h
	main.c
)