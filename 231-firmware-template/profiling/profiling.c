#include "profiling.h"

#include "pico/stdlib.h"


typedef struct {
  const char* name;
  uint32_t count;
  uint32_t previous_us;
  uint32_t avg_sum;
  uint32_t max_us;
} profiling_stopwatch_t;

static profiling_stopwatch_t profiling_stopwatches[profiling_stopwatch_count];

void profiling_stopwatch_init(uint32_t id, const char* name) {
  if (id >= profiling_stopwatch_count) {
    return;
  }
  profiling_stopwatches[id].name = name;
  profiling_stopwatches[id].count = 0;
  profiling_stopwatches[id].previous_us = 0;
  profiling_stopwatches[id].avg_sum = 0;
  profiling_stopwatches[id].max_us = 0;
}

void profiling_start(uint32_t id) {
  profiling_stopwatches[id].previous_us = time_us_32();
}

void profiling_stop(uint32_t id) {
  profiling_stopwatches[id].count++;
  uint32_t iteration_us = time_us_32() - profiling_stopwatches[id].previous_us;
  // profiling_stopwatches[id].previous_us = now_us;

  // update max_us
  if (iteration_us > profiling_stopwatches[id].max_us) {
    profiling_stopwatches[id].max_us = iteration_us;
  }
  // update avg_sum
  profiling_stopwatches[id].avg_sum =
      profiling_stopwatches[id].avg_sum + iteration_us;
}

bool profiling_get(uint32_t id, profiling_result_t* result) {
  if (profiling_stopwatches[id].name == NULL) {
    return false;
  }
  result->name = profiling_stopwatches[id].name;
  result->count = profiling_stopwatches[id].count;
  result->avg_us = (float)profiling_stopwatches[id].avg_sum /
                   profiling_stopwatches[id].count;
  result->max_us = profiling_stopwatches[id].max_us;
  return true;
}

void profiling_reset(void) {
  for (uint i = 0; i < profiling_stopwatch_count; i++) {
    profiling_stopwatches[i].count = 0;
    //profiling_stopwatches[i].previous_us = 0;
    profiling_stopwatches[i].avg_sum = 0;
    profiling_stopwatches[i].max_us = 0;
  }
}