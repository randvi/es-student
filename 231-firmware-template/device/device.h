#pragma once

#include <stdint.h>
#include "pico/version.h"

#define DEVICE_NAME "es-cmd-module"
#define FIRMWARE_VERSION "2.3.5"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/randvi/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

typedef struct {
  char board[10];
  char board_id[17]; // serial , pico_get_unique_board_id_string
  uint32_t manufacturer;
  uint32_t part;
  uint8_t revision;
  char pico_sdk[sizeof(PICO_SDK_VERSION_STRING) + 1];
} device_info_t;


void device_get_info(device_info_t* info);

