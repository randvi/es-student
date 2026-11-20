cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(231_firmware_template)

pico_sdk_init()

add_compile_options(-Wall)

add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)
######
# железо прибора: единственное место проекта с SDK
add_library(platform STATIC
    platform/platform.c
)

target_include_directories(platform PUBLIC
     ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

target_link_libraries(platform PRIVATE
    pico_stdlib
)

# путь к заголовкам — папка проекта, platform подключает pico_stdlib как PRIVATE;
# через этот путь logic находит и firmware.h

# логика прибора: задачи и команды, без SDK

add_library(logic STATIC
    logic/api/api.c
    logic/api/api-callbacks.c
    logic/led-task/led-task.c
    logic/button-task/button-task.c
    logic/pi-task/pi-task.c
)
  # исходники api, led-task, button-task и pi-task

target_include_directories(logic 
    PUBLIC  
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/logic/api
    ${CMAKE_CURRENT_SOURCE_DIR}/logic/led-task
    ${CMAKE_CURRENT_SOURCE_DIR}/logic/button-task
    ${CMAKE_CURRENT_SOURCE_DIR}/logic/pi-task
)

target_link_libraries(logic 
    PRIVATE 
      platform
      pico_stdlib
      systime
      logging
      profiling
      device
    PUBLIC
      stdio_text_protocol
)

# путь к заголовкам — папка logic
# stdio_text_protocol — PUBLIC, остальные библиотеки — PRIVATE


##########

target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

add_executable(${PROJECT_NAME}
    main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    device
    logging
    profiling
    stdio_text_protocol
    systime
    platform
    logic
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
