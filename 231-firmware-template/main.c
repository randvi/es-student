/*  cmake -G "Unix Makefiles" ..
    Copy-Item .\231_firmware_template.uf2 D:\
    arm-none-eabi-size .\231_firmware_template.elf.
*/

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "firmware.h"
#include "api.h"
#include "button-task.h"
#include "device/device.h"
#include "led-task.h"
#include "logging/log.h"
#include "pi-task.h"

#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"


enum {
  STOPWATCH_LOOP,
  STOPWATCH_COMMAND,
  STOPWATCH_LED,
  STOPWATCH_BUTTON,
  STOPWATCH_PI
} stopwatch_name;

int main() {
  // stdio
  stdio_init_all();

  LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);


  profiling_stopwatch_init(STOPWATCH_LOOP, "loop");
  profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
  profiling_stopwatch_init(STOPWATCH_LED, "led");
  profiling_stopwatch_init(STOPWATCH_BUTTON, "button");
  profiling_stopwatch_init(STOPWATCH_PI, "pi");

  stdio_text_protocol_init();
  led_task_init();
  button_task_init();

  while (1) {

    profiling_start(STOPWATCH_LOOP);

    profiling_start(STOPWATCH_COMMAND);
    const command_t* command = stdio_text_protocol_handle();
    if (command != NULL) {
      api_handle(command);
    }
    profiling_stop(STOPWATCH_COMMAND);

    profiling_start(STOPWATCH_LED);
    led_task_handle();
    profiling_stop(STOPWATCH_LED);


    profiling_start(STOPWATCH_BUTTON);
    button_task_handle();
    profiling_stop(STOPWATCH_BUTTON);
    
    profiling_start(STOPWATCH_PI);
    pi_task_handle();
    profiling_stop(STOPWATCH_PI);

    profiling_stop(STOPWATCH_LOOP);

  }
}
