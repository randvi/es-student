#include "button-task.h"

#include "platform.h"
#include "systime/systime.h"

typedef enum {
  BUTTON_STATE_RELEASED,
  BUTTON_STATE_PRESS,
  BUTTON_STATE_PRESSED,
  BUTTON_STATE_RELEASE,
} button_state_t;

const unsigned int DEBOUNCE_US = 20000;

static button_state_t button_state;
static uint32_t presses = 0;
static uint64_t last_button_action_us;
static button_task_callback_t button_task_on_press;

void button_task_init(button_task_callback_t on_press) {
  button_task_on_press = on_press;
  button_state = BUTTON_STATE_RELEASED;
}

static bool period_elapsed(uint64_t* last_us) {
  uint64_t now_us = systime_us();

  if (now_us - *last_us < DEBOUNCE_US) {
    return false;
  }

  return true;
}

void button_task_handle(void) {
  // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
  // читаем часы
  bool state = platform_button_read(); 
  uint64_t now_us = systime_us();

  switch (button_state) {
    // переходы по таблице; при переходе в «нажата»
    // увеличиваем счётчик и вызываем led_task_next_state()
    case BUTTON_STATE_RELEASED:
      if (!state) {
        last_button_action_us = now_us;
        button_state = BUTTON_STATE_PRESS;
      }
      break;
    case BUTTON_STATE_PRESS:
      if (state) {
        button_state = BUTTON_STATE_RELEASED;
      } else {
        if (period_elapsed(&last_button_action_us)) {
          button_state = BUTTON_STATE_PRESSED;
          presses++;
          button_task_on_press(); //led_task_next_state();
        }
      }
      break;
    case BUTTON_STATE_PRESSED:
      if (state) {
        last_button_action_us = now_us;
        button_state = BUTTON_STATE_RELEASE;
      }
      break;

    case BUTTON_STATE_RELEASE:
      if (!state) {
        button_state = BUTTON_STATE_PRESSED;
      } else {
        if (period_elapsed(&last_button_action_us)) {
          button_state = BUTTON_STATE_RELEASED;
        }
      }
      break;
  }
}

uint32_t button_task_get_press_count() { return presses; }

bool button_task_is_pressed() {
  return (button_state == BUTTON_STATE_PRESSED ||
          button_state == BUTTON_STATE_RELEASE);
}
