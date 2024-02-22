cmake_minimum_required(VERSION 3.20)
project(WallShell C)

set(CMAKE_C_STANDARD 99 REQUIRED)

#configure doing this or using wallshell_config.h
# add_definitions(-DNO_BASIC_COMMANDS)
# add_definitions(-DDISABLE_MALLOC)

add_executable(WallShell
		wall_shell.h
		wall_shell.c
		wallshell_config.h
		main.c
)
