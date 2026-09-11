cmake_minimum_required(VERSION 3.16)
project(AlienShooterPort VERSION 1.2.9 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(TARGET_NAME "alienport")

# Architecture flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    set(ARCH_FLAGS "-march=armv8-a")
    set(PLATFORM_DEFINES "-D__LINUX__" "-D__ARM64__")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(ARCH_FLAGS "")
    set(PLATFORM_DEFINES "-D__LINUX__")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ffunction-sections -fdata-sections -Wall -Wno-unused-function -Wno-format-truncation ${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections ${ARCH_FLAGS}")

# Sources
set(SOURCES
    src/main.c
    src/so_util.c
    src/imports.gen.c
    src/android_shim.c
    src/asset_shim.c
    src/bionic_shims.c
    src/jni_shim.c
    src/egl_shim.c
    src/opensles_shim.c
    src/pthread_bridge.c
)

add_executable(${TARGET_NAME} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PRIVATE ${PLATFORM_DEFINES} _GNU_SOURCE)
target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    /usr/include/aarch64-linux-gnu/SDL2
    /usr/include/aarch64-linux-gnu
)

# Dependencies
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)

target_include_directories(${TARGET_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_compile_options(${TARGET_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
target_link_directories(${TARGET_NAME} PRIVATE ${SDL2_LIBRARY_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    EGL
    GLESv2
    m
    dl
    z
    pthread
)
