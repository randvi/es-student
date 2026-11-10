/*  cmake -G "Unix Makefiles" ..
    Copy-Item .\231_firmware_template.uf2 D:\
*/

#include <stdio.h>
#include <string.h>

#include "device.h"
#include "log.h"
#include "pico/stdlib.h"
#include "profiling.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "led-task.h"


/*
void blink(void) {
  uint64_t now_us = time_us_64();

  if (now_us - last_toggle_us >= BLINK_HALF_PERIOD_MS * 1000) {
    last_toggle_us = now_us;
    led_toggle();
  }
}
*/
int main() {
  // stdio
  stdio_init_all();
  
  //profiling_init();
  stdio_text_protocol_init();
  led_task_init();

  while (1) {
    
    led_task_handle();
    const command_t* command = stdio_text_protocol_handle();
    if (command != NULL) {

      api_handle(command);

    }
  }
}
