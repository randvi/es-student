#include "stdio-text-protocol.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint line_length = 0;

static command_t* command;

void stdio_text_protocol_init(void) {
  command = (command_t*)malloc(sizeof(command_t));
  command->name = NULL;
  command->argc = 0;
  command->truncated = false;
  for (int i = 0; i < STDIO_TEXT_PROTOCOL_MAX_ARGS; i++) {
    command->argv[i] = NULL;
  }
}

// Читает один символ, не ожидая его. Когда строка закончена, возвращает
// команду, иначе NULL. Команда действительна до следующего вызова.

const command_t* stdio_text_protocol_handle(void) {
  // читаем символ без ожидания; нет символа — возвращаем NULL
  int symbol = getchar_timeout_us(0);

  if (symbol == PICO_ERROR_TIMEOUT) {
    return NULL;
  }

  // конец строки: пустую строку пропускаем, иначе переводим строку в терминале,
  // делим строку на слова и возвращаем команду
  if (symbol == '\r' || symbol == '\n') {
    putchar('\n');
    if (line_length == 0) {
      return NULL;
    } else {  // делим строку
      line[line_length] = '\0';
      // parsing and filling command
      command->argc = 0;
      command->name = NULL;
      char* p = line;
      //
      while (*p == ' ') {
        *p = '\0';
        p++;
      }
      // empty line - the end of input line
      if (*p == '\0') {
        return NULL;
      }

      command->name = p;
      bool expect_new_word = false;

      while (*p != '\0') {
        if (*p == ' ') {
          *p = '\0';
          expect_new_word = true;
          p++;
        } else if (expect_new_word) {
          // first symbol new word
          if (command->argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
            command->argv[command->argc] = p;
            command->argc++;
            expect_new_word = false;  // start wors
            p++;
          } else {
            command->truncated = true;
            break;
          }
        } else {
          p++;  // next symbol
        }
      }  // // while

      line_length = 0;
      return command;
    }  // else if line_length != 0
  }

  // Backspace (0x08 или 0x7f): убираем последний символ и стираем его на
  // экране
  // "\b \b"
  if (symbol == 0x08 || symbol == 0x7f) {
    if (line_length > 0) {
      line_length--;
    }
    putchar('\b');
    putchar(' ');
    putchar('\b');

    return NULL;
  }

  // печатный символ: кладём в буфер и повторяем в терминал;
  if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE) {
    line[line_length] = (char)symbol;
    line_length = line_length + 1;
    putchar(symbol);
    command->truncated = false;
  } else {
    // буфер полон — помечаем строку как переполненную
    command->truncated = true;
  }

  return NULL;
}