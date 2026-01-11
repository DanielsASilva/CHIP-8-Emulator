cmake_minimum_required(VERSION 3.15...4.2)

project(chip8)   

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(chip8 main.cpp src/chip8.cpp headers/chip8.h)

target_link_libraries(chip8 PRIVATE SDL3::SDL3)
target_include_directories(chip8 PRIVATE headers)
