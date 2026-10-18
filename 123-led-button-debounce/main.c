/*  cmake -G "Unix Makefiles" ..
    Copy-Item .\123_led_button.uf2 D:\
*/
#include "hardware/gpio.h"
#include "pico/stdlib.h"

const uint LED_PIN = 25;
const uint BUTTON_PIN = 1;
const uint DEBOUNCE_MS = 20;

bool get_button_debounce(uint pin) {
  bool state = gpio_get(pin);
  sleep_ms(DEBOUNCE_MS);
  return state && gpio_get(pin);
}


int main() {
  // led
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  // button
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  bool led = false;
  bool previous = false;

  while (1) {
    bool current = get_button_debounce(BUTTON_PIN);

    if (previous == true && current == false) {
      led = !led;
      gpio_put(LED_PIN, led);
    }

    previous = current;
  }
}

