cmake_minimum_required(VERSION 3.29)
project(Chip8Emulator)

# needed for zed
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/SDL)

include_directories(
    include

    external/SDL/include
)

add_executable(
    ${PROJECT_NAME}
    src/main.c
    src/chip8.c
)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
