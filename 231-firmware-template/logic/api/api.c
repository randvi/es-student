#include "api.h"
#include "api-commands.h"

#include <stdio.h>
#include <string.h>


static void help() {
  printf("%-16s %s\n", "help", "list of commands");
  for (uint32_t i = 0; i < api_command_count; i++) {
    printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
  }
}

void api_handle(const command_t* command) {
  // truncated
  if (command->truncated == true) {
    printf("error: command longer than 63 characters or 4 arguments\n");
    return;
  }
  // help
  if (strcmp(command->name, "help") == 0) {
    help();
    return;
  }
  // looking up in the table commands
  for (uint32_t i = 0; i < api_command_count; i++) {
    if (strcmp(command->name, api_commands[i].name) == 0) {
      api_commands[i].callback(command);
      return;
    }
  }

  // not found - unknown
  printf("error: unknown command '%s', try help\n", command->name);
}