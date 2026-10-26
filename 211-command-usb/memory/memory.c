
#include "memory.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "command.h"
#include "device.h"
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

int main(void);

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

uint32_t data_variable = 100;
uint32_t bss_variable;

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

void fw_info(void) {
  // считаем вызов: data_variable и bss_variable на единицу больше
  data_variable++;
  bss_variable++;

  // адреса функций со сброшенным признаком Thumb
  uint16_t* main_code = (uint16_t*)((uintptr_t)main & ~1u);
  uint16_t* fw_info_code = (uint16_t*)((uintptr_t)fw_info & ~1u);

  // локальная переменная и блок из кучи
  uint32_t stack_variable = 1946;
  uint32_t* heap_variable = malloc(sizeof(uint32_t));
  if (heap_variable != NULL) {
    *heap_variable = 1951;
  }

  // шапка: объект, адрес, значение
  printf("%-15s %-10s %-10s\n", "object", "address ", "value");
  // main, fw_info  — адрес с признаком Thumb и два байта по сброшенному адресу
  printf("%-15s 0x%08x 0x%x\n", "main", (void*)main, *main_code);
  printf("%-15s 0x%08x 0x%x\n", "fw_info", (void*)fw_info, *fw_info_code);
  // commands       — адрес массива
  const struct command_t* p_commands = commands;
  printf("%-15s 0x%08x\n", "commands", &commands);
  // обработчики    — имя команды и адрес обработчика, строкой на команду
  for (uint i = 0; i < command_count; i++) {
    printf("- %-13s 0x%08x\n", commands[i].name, (void*)commands[i].handler);
  }
  // константы      — адрес и значение строк паспорта из device.h
  printf("%-15s 0x%08x %s\n", "DEVICE_PROJECT", DEVICE_PROJECT, DEVICE_PROJECT);
  printf("%-15s 0x%08x %s\n", "DEVICE_BOARD", DEVICE_BOARD, DEVICE_BOARD);
  // data_variable  — адрес и значение, секция .data
  printf("%-15s 0x%08x %u\n", "data_variable", &data_variable, data_variable);
  // bss_variable   — адрес и значение, секция .bss
  printf("%-15s 0x%08x %u\n", "bss_variable", &bss_variable, bss_variable);
  // stack_variable — адрес и значение
  printf("%-15s 0x%08x %u\n", "stack_variable", &stack_variable,
         stack_variable);
  // heap_variable  — адрес и значение
  printf("%-15s 0x%08x %u\n", "heap_variable", heap_variable, *heap_variable);
  // возвращаем блок кучи
  free(heap_variable);
}