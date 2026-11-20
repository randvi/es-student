#include "led-task.h"

#include "platform.h"
#include "systime/systime.h"

static led_state_t led_state = LED_STATE_OFF;
static uint64_t half_period_us;
static uint64_t last_toggle_us;
static bool led_on = false;

void led_task_init(void) {
  // настраиваем период 1000 мс, состояние «мигает»
  led_task_set_period_ms(1000);
  led_state = LED_STATE_BLINK;
  last_toggle_us = 0;
}

void led_task_set_state(led_state_t state) {
  // запоминаем состояние и выполняем действие при входе:
  // «выключен» — гасим, «горит» — зажигаем, «мигает» — первое переключение
  // сразу
  led_state = state;

  switch (led_state) {
    case LED_STATE_OFF:
      led_on = false;
      platform_led_set(led_on);
      break;
    case LED_STATE_ON:
      led_on = true;
      platform_led_set(led_on);
      break;
    case LED_STATE_BLINK:
      // Первое переключение — на ближайшей итерации
      last_toggle_us = 0;
      break;
  }
}

led_state_t led_task_get_state(void) { return led_state; }

void led_task_handle(void) {
  switch (led_state) {
    // «выключен» и «горит» — ничего не делаем, светодиод установлен при входе
    // «мигает» — каждые полпериода переключаем светодиод
    case LED_STATE_OFF:
    case LED_STATE_ON:
      // Светодиод установлен при входе в состояние, делать нечего
      break;
    case LED_STATE_BLINK:
      if (systime_period_elapsed(&last_toggle_us, half_period_us)) {
        led_on = !led_on;
        platform_led_set(led_on);
      }
      break;
  }
}

bool led_task_set_period_ms(uint32_t period_ms) {
  // С нулевым полупериодом светодиод переключался бы
  // на каждой итерации: такой период не принимается
  if (period_ms == 0) {
    return false;
  }

  half_period_us = (uint64_t)period_ms * 1000 / 2;
  return true;
}

uint32_t led_task_get_period_ms(void) { return (half_period_us * 2) / 1000; }

void led_task_next_state(void) {
  led_state = (led_state + 1) % 3;
  led_task_set_state(led_state);
}