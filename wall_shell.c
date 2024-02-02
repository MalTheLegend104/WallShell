/** 
 * @file command_handler.h
 * @author MalTheLegend104
 * @brief C99 compliant command handler. Meant to be easily portable and highly configurable.
 * @version v1.0
 */
#include "wall_shell.h"

/**
 * @brief Operator overloading isn't available in C. Compare two commands using this.
 *
 * @param c1 Command to compare.
 * @param c2 Command to compare.
 * @return true If they are the same.
 * @return false If they are different.
 */
bool compareCommands(const command_t c1, const command_t c2) {
	if (c1.aliases != c2.aliases) return false;
	if (c1.aliases_count != c2.aliases_count) return false;
	if (strcmp(c1.commandName, c2.commandName) != 0) return false;
	if (c1.helpCommand != c2.helpCommand) return false;
	if (c1.mainCommand != c2.mainCommand) return false;
	return true;
}

/**
 * @brief Register the command to the command handler.
 * @param c Command to be registered.
 */
void registerCommand(const command_t c) {

}

void deregisterCommand(const command_t c) {

}

void executeCommand(char* commandBuf) {

}


void printGeneralHelp(help_entry_general_t* entry) {

}
void printSpecificHelp(help_entry_specific_t* entry) {

}

void terminalMain() {

}