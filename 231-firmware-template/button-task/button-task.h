#pragma once

#include <stdint.h>

void button_task_init(void);
void button_task_handle(void);
uint32_t button_task_get_press_count();
bool button_task_is_pressed();