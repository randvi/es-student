#include "device/device.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/unique_id.h"
#include "pico/version.h"

void device_get_info(device_info_t* info) {
  strcpy(info->board, DEVICE_BOARD);
  pico_get_unique_board_id_string(info->board_id, 17);
  volatile uint32_t* chip_id =
      (uint32_t*)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
  uint32_t id = *chip_id;

  info->manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >>
                       SYSINFO_CHIP_ID_MANUFACTURER_LSB;

  info->part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;

  info->revision =
      (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;
  strcpy(info->pico_sdk, PICO_SDK_VERSION_STRING);
}
