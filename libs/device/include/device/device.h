#pragma once

#include <stdint.h>

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

typedef struct {
  char board[10];
  char board_id[17]; // serial , pico_get_unique_board_id_string
  uint32_t manufacturer;
  uint32_t part;
  uint8_t revision;
  char pico_sdk[10];
} device_info_t;


void device_get_info(device_info_t* info);

