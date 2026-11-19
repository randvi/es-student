cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

project(profiling)

add_library(${PROJECT_NAME} STATIC
    src/profiling.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
)
