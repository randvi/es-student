#include "led-task.h"

#include "pico/stdlib.h"

const uint LED_PIN = 25; // 9

static led_state_t led_state = LED_STATE_OFF;
static uint64_t half_period_us;
static uint64_t last_toggle_us;

static bool period_elapsed(uint64_t* last_us, uint64_t period_us) {
  uint64_t now_us = time_us_64();

  if (now_us - *last_us < period_us) {
    return false;
  }

  // Следующее срабатывание отсчитывается от прошлого, а не от «сейчас»
  *last_us += period_us;

  // Отстали больше чем на период: не догонять, а начать отсчёт заново
  if (now_us - *last_us >= period_us) {
    *last_us = now_us;
  }

  return true;
}

void led_task_init(void) {
  // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
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
      gpio_put(LED_PIN, false);
      break;
    case LED_STATE_ON:
      gpio_put(LED_PIN, true);
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
      if (period_elapsed(&last_toggle_us, half_period_us)) {
        gpio_put(LED_PIN, !gpio_get(LED_PIN));
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