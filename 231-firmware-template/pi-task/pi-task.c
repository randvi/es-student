#include "pi-task.h"

#include "pico/stdlib.h"

#define PI_TERMS_PER_CALL 100

typedef enum {
    PI_STATE_NOT_STARTED,
    PI_STATE_CALCULATING,
    PI_STATE_DONE,
} pi_state_t;

static pi_state_t pi_state = PI_STATE_NOT_STARTED ;
static uint64_t pi_task_start_us;
static uint64_t pi_task_end_us;

// Состояние расчёта: должно пережить возврат из обработчика
bool pi_running = false;
bool pi_ready = false;
static int pi_k;
static int pi_n;
static double pi_sum;
static double pi_sign;
static double pi_result;

bool pi_task_start(int n) {
  if (pi_state == PI_STATE_CALCULATING) {
    return false;
  }
  pi_task_start_us = time_us_64();
  pi_n = n;
  pi_k = 0;
  pi_sum = 0.0;
  pi_sign = 1.0;
  pi_state = PI_STATE_CALCULATING;
  return true;
}

void pi_task_handle(void) {
  if (pi_state != PI_STATE_CALCULATING) {
    return;
  }

  // Очередная порция членов ряда, но не дальше последнего
  int end = pi_k + PI_TERMS_PER_CALL;
  if (end < pi_k) {  // overflow
    end = pi_n;
  }
  if (end > pi_n) { // more ten last
    end = pi_n;
  }
  for (; pi_k < end; pi_k++) {
    pi_sum += pi_sign / (2.0 * pi_k + 1.0);
    pi_sign = -pi_sign;
  }

  if (pi_k == pi_n) {  // the end
    pi_result = 4.0 * pi_sum;
    pi_state = PI_STATE_DONE;
    pi_task_end_us = time_us_64();
  }
}

// Готов ли результат: если да, записать его по указателю
bool pi_task_get_result(double* result, uint64_t* time_result) {
  if (pi_state != PI_STATE_DONE) {
    return false;
  }
  *result = pi_result;
  *time_result = pi_task_end_us - pi_task_start_us;
  return true;
}

bool pi_task_is_started() {
  return pi_state != PI_STATE_NOT_STARTED;
}