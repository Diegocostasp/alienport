cmake_minimum_required(VERSION 3.10)
project(alienport C)

set(CMAKE_C_STANDARD 11)

# Find SDL2 (Note: In cross-compilation, this might need manual paths, 
# but for now we assume the toolchain/env handles it)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)

set(SOURCES
    src/main.c
    src/so_util.c
    src/android_bridge.c
)

add_executable(alienport ${SOURCES})
target_link_libraries(alienport ${SDL2_LIBRARIES} dl pthread m)

# Link against GLESv2
target_link_libraries(alienport GLESv2)
