#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "device.h"
#include "pico/time.h"
#include "profiling.h"

typedef void (*api_callback_t)(const command_t* command);

typedef struct {
  const char* name;
  const char* help;
  api_callback_t callback;
} api_command_t;

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585
// тактов; 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
static const uint CALC_PI_TERMS = 1000000;

volatile double pi_result;

static bool parse_u32(const char* text, uint32_t* value) {
  // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
  if (!isdigit((unsigned char)text[0])) {
    return false;
  }

  char* end;
  errno = 0;
  unsigned long number = strtoul(text, &end, 10);

  // после числа остались символы или число не поместилось
  if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) {
    return false;
  }

  *value = (uint32_t)number;
  return true;
}

static double calc_pi(int n) {
  double sum = 0.0;
  double sign = 1.0;

  for (int k = 0; k < n; k++) {
    // Прибавить очередной член ряда: ±1 / (2k + 1)
    sum += sign / (2.0 * k + 1.0);
    sign = -sign;
  }
  // Сумма ряда равна π/4
  return 4.0 * sum;
}

static void cmd_calc_pi(const command_t* command) {
  uint32_t terms = CALC_PI_TERMS;

  if (command->argc != 0 && (command->argc > 1 || !parse_u32(command->argv[0], &terms))) {

    printf("error: usage calc_pi [terms]\n");
    return;
  }

  if (terms == 0) {
    printf("error: terms must be greater than 0\n");
    return;
  }
  
  
  uint64_t start_us = time_us_64();
  pi_result = calc_pi(terms);
  uint64_t spent_us = time_us_64() - start_us;

  printf("pi: %.9f\n", pi_result);
  printf("time: %llu ms\n", spent_us / 1000);
}

static void cmd_info(const command_t* command) {
  // печатаем паспорт устройства
  device_info();
}

static void cmd_uptime(const command_t* command) {
  printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void cmd_main_time_exec(const command_t* command) {
  printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(),
         (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t* command) {
  profiling_reset_max();
  printf("max reset\n");
}

const api_command_t api_commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_exec", "superloop iteration: average and maximum",
     cmd_main_time_exec},
    {"main_time_reset", "reset maximum iteration", cmd_main_time_reset},
};

const uint api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

void static help() {
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