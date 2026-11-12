#pragma once

#include <stdint.h>

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE,
} button_state_t;

void button_task_init(void);
void button_task_handle(void);
uint32_t button_task_get_press_count();
bool button_task_is_pressed();