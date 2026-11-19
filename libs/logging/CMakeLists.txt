cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

project(logging)

add_library(${PROJECT_NAME} STATIC
    src/log.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)
