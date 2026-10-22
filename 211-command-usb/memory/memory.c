
#include "memory.h"

#include <stdint.h>
#include <stdio.h>

#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

#define PICO_SRAM_SIZE_BYTES 264 * 1024
#define PICO_ROM_SIZE_BYTES 16 * 1024

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

static void row(const char* name, uintptr_t start, uintptr_t end) {
  printf("%-10s 0x%08x 0x%08x %8u\n", name, (unsigned)start, (unsigned)end,
         (unsigned)(end - start));
}

void mem_info(void) {
  // шапка таблицы: область, начало, конец, размер
  printf("%-10s %-10s %-10s %-10s\n", "area", "start", "end", "size");

  // flash — XIP_BASE и PICO_FLASH_SIZE_BYTES
  row("flash", (uintptr_t)XIP_BASE,
      (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
  // sram — базовый адрес из SDK, размер из datasheet
  row("sram", (uintptr_t)SRAM_BASE,
      (uintptr_t)(SRAM_BASE + PICO_SRAM_SIZE_BYTES));
  // rom — базовый адрес из SDK, размер из datasheet
  row("rom", (uintptr_t)ROM_BASE, (uintptr_t)(ROM_BASE + PICO_ROM_SIZE_BYTES));
  // image — от __flash_binary_start до __flash_binary_end
  row("image", (uintptr_t)&__flash_binary_start,
      (uintptr_t)&__flash_binary_end);
  // free  — от __flash_binary_end до конца флеш-памяти
  row("free", (uintptr_t)&__flash_binary_end,
      (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
  // boot2 — от __boot2_start__ до __boot2_end__
  row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
  // text  — от __boot2_end__ до __etext: код и константы
  row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);
  // data flash — хранение .data, от __etext, длиной с .data
  row("data flash", (uintptr_t)&__etext, (uintptr_t)&__flash_binary_end);
  // data ram   — работа .data, от __data_start__ до __data_end__
  row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
  // bss        — от __bss_start__ до __bss_end__
  row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
  // heap       — от __bss_end__ до __HeapLimit
  row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
  // stack      — от __StackBottom до __StackTop
  row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

  printf("\ntotal\n");
  // итог: образ во флеш и из чего он сложился
  uint flash_image_size = (unsigned)((uintptr_t)&__flash_binary_end -
                                     (uintptr_t)&__flash_binary_start);
  uint boot2_size =
      (unsigned)((uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__);
  uint text_size = (unsigned)((uintptr_t)&__etext - (uintptr_t)&__boot2_end__);
  uint data_size =
      (unsigned)((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__);

  printf("  %-11s %8u = boot2 %u + text %u + data %u \n", "flash image",
         flash_image_size, boot2_size, text_size, data_size);

  // итог: свободно во флеш-памяти из всего её объёма
  printf("  %-11s %8u of%8u\n", "flash free",
         (unsigned)((uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES) -
                    (uintptr_t)&__flash_binary_end),
         (unsigned)PICO_FLASH_SIZE_BYTES);

  // итог: занято в ОЗУ — .data и .bss
  uint bss_size =
      (unsigned)((uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__);
  printf("  %-11s %8u = data %u + bss %u\n", "ram used", data_size + bss_size,
         data_size, bss_size);

  // итог: свободно в ОЗУ — под кучу и под стек
  uint stack_size =
      (unsigned)((uintptr_t)&__StackTop - (uintptr_t)&__StackBottom);
  uint heap_size =
      (unsigned)((uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__);
  printf("  %-11s %8u for heap and %u for stack \n", "ram free", heap_size,
         stack_size);
}