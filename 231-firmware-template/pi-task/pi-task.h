#pragma once

#include <stdbool.h>
#include <stdint.h>

bool pi_task_start(int n);
void pi_task_handle(void);
bool pi_task_get_result(double* result, uint64_t* time_result);
bool pi_task_is_started();