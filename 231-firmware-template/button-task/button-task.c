#include "button-task.h"

#include "led-task.h"
#include "pico/stdlib.h"

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE,
} button_state_t;

const uint BUTTON_PIN = 15;  // 11
const uint DEBOUNCE_US = 20000;

static button_state_t button_state;
static uint32_t presses = 0;
static uint64_t last_button_action_us;

void button_task_init(void) {
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);
  button_state = BUTTON_STATE_RELEASED;
}

static bool period_elapsed(uint64_t* last_us) {
  uint64_t now_us = time_us_64();

  if (now_us - *last_us < DEBOUNCE_US) {
    return false;
  }

  return true;
}

void button_task_handle(void) {
  // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
  // читаем часы
  bool state = gpio_get(BUTTON_PIN);
  uint64_t now_us = time_us_64();

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
          led_task_next_state();
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
