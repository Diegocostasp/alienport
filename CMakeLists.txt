cmake_minimum_required(VERSION 3.16)
project(AlienShooterPort VERSION 1.0.0 LANGUAGES C)

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O2 -ffunction-sections ${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g ${ARCH_FLAGS}")

# Sources
set(SOURCES
    src/main.c
    src/so_util.c
    src/imports.gen.c
    src/android_shim.c
    src/jni_shim.c
    src/egl_shim.c
    src/opensles_shim.c
    src/util.c
    src/error.c
)

add_executable(${TARGET_NAME} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PRIVATE ${PLATFORM_DEFINES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Dependencies via pkg-config
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(GLESV2 REQUIRED IMPORTED_TARGET glesv2)

target_link_libraries(${TARGET_NAME} PRIVATE
    PkgConfig::SDL2
    PkgConfig::EGL
    PkgConfig::GLESV2
    m
    dl
    z
    pthread
    stdc++
)
