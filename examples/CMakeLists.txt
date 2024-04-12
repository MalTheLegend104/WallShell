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