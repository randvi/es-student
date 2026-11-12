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
#include "button-task.h"

int main() {
  // stdio
  stdio_init_all();
  
  //profiling_init();
  stdio_text_protocol_init();
  led_task_init();
  button_task_init();

  while (1) {
    
    led_task_handle();
    button_task_handle();
    const command_t* command = stdio_text_protocol_handle();
    if (command != NULL) {

      api_handle(command);

    }
  }
}
