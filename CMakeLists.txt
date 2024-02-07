cmake_minimum_required(VERSION 3.20)
project(WallShell C)

set(CMAKE_C_STANDARD 99)

add_executable(WallShell
		main.c
		wall_shell.h
		wall_shell.c
		wallshell_config.h
)
