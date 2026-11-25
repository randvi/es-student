#include "api-commands.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "button-task/button-task.h"
#include "device/device.h"
#include "firmware.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "profiling/profiling.h"
#include "systime/systime.h"

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585
// тактов; 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
static const uint32_t CALC_PI_TERMS = 1000000;

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

static void cmd_pi_start(const command_t* command) {
  uint32_t terms = CALC_PI_TERMS;

  if (command->argc != 0 &&
      (command->argc > 1 || !parse_u32(command->argv[0], &terms))) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }

  if (terms == 0) {
    printf("error: terms must be greater than 0\n");
    return;
  }

  if (pi_task_start(terms)) {
    printf("pi: started, %lu terms\n", terms);
  } else {
    printf("error: pi is busy\n");
  }
}

static void cmd_pi(const command_t* command) {
  if (!pi_task_is_started()) {
    printf("pi: not started\n");
    return;
  }
  double pi_res;
  uint64_t pi_spend_us;
  if (pi_task_get_result(&pi_res, &pi_spend_us)) {
    printf("pi: %.9f in %llu ms\n", pi_res, pi_spend_us / 1000);
  } else {
    printf("pi: running\n");
  }
}

static void cmd_info(const command_t* command) {
  // печатаем паспорт устройства
  device_info_t device_card;
  device_get_info(&device_card);

  printf("name: %s\n", FIRMWARE_NAME);
  printf("version: %s\n", FIRMWARE_VERSION);
  printf("project: %s\n", FIRMWARE_PROJECT);
  printf("repo: %s\n", FIRMWARE_REPO);
  printf("board: %s\n", device_card.board);
  printf("serial: %s\n", device_card.board_id);
  printf("chip: manufacturer 0x%03lx, part 0x%04lx, revision %u\n",
         device_card.manufacturer, device_card.part, device_card.revision);
  printf("pico-sdk: %s\n", device_card.pico_sdk);
}

static void cmd_uptime(const command_t* command) {
  printf("uptime: %llu ms\n", systime_ms());
}

static void print_led_state(void) {
  led_state_t led_state = led_task_get_state();
  printf("led: %s, period %lu ms\n",
         (led_state == LED_STATE_ON)
             ? "on"
             : ((led_state == LED_STATE_OFF) ? "off" : "blink"),
         led_task_get_period_ms());
}

static void cmd_led_enable(const command_t* command) {
  led_task_set_state(LED_STATE_ON);
  print_led_state();
}

static void cmd_led_disable(const command_t* command) {
  led_task_set_state(LED_STATE_OFF);
  print_led_state();
}

static void cmd_led_blink(const command_t* command) {
  led_task_set_state(LED_STATE_BLINK);
  print_led_state();
}

static void cmd_led_period(const command_t* command) {
  uint32_t period_ms;

  // parsing
  if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms)) {
    printf("error: usage led_period <period_ms>\n");
    return;
  }

  if (!led_task_set_period_ms(period_ms)) {
    printf("error: period_ms must be greater than 0\n");
    return;
  }
  // ответ: состояние и период светодиода
  print_led_state();
}

static void cmd_button(const command_t* command) {
  printf("button: %s, presses %lu\n",
         button_task_is_pressed() ? "pressed" : "released",
         button_task_get_press_count());
}

static void cmd_profiling(const command_t* command) {
  if (PROFILING_ENABLED == 0) {
    printf("profiling is disabled in this build\n");
    return;
  }
  profiling_result_t result;
  for (uint32_t i = 0; i < profiling_stopwatch_count; i++) {
    if (profiling_get(i, &result)) {
      printf("%-12s count %-12lu mean %.2f us  max %lu us \n", result.name,
             result.count, result.avg_us, result.max_us);
    }
  }
}

static void cmd_profiling_reset(const command_t* command) {
   profiling_reset(); 
   printf("profiling reset\n");
  }

const api_command_t api_commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"pi_start", "start calculate pi: pi_start [<terms>]", cmd_pi_start},
    {"pi", "get pi result or state", cmd_pi},
    {"led_enable", "set the led state on", cmd_led_enable},
    {"led_disable", "set the led state off", cmd_led_disable},
    {"led_blink", "set the led state blink", cmd_led_blink},
    {"led_period", "set led period blink", cmd_led_period},
    {"button", "button state and number of presses", cmd_button},
    {"profiling", "shows all started stopwatch results since reset",
     cmd_profiling},
    {"profiling_reset", "reset all stopwatch", cmd_profiling_reset},

};


const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

