#pragma once

#include "pico/types.h"

#define profiling_stopwatch_count  5

typedef struct {
  const char* name;
  uint32_t count;
  float avg_us;
  uint32_t max_us;

} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char* name);

void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);

// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);