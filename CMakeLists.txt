cmake_minimum_required(VERSION 3.16)
project(raycaster CXX)

# --- Language standard -------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release-ish build if the user didn't pick one. A raycaster is
# performance work; an accidental unoptimized build would make our later FPS
# measurements meaningless.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

# --- Dependencies ------------------------------------------------------------
# Ubuntu's libsdl2-dev ships CMake config files, so find_package works directly.
find_package(SDL2 REQUIRED)

# --- Target ------------------------------------------------------------------
add_executable(raycaster
    src/main.cpp
)

target_include_directories(raycaster PRIVATE src)
target_link_libraries(raycaster PRIVATE SDL2::SDL2 SDL2::SDL2main)

# Warnings on: this is a learning project, so surface everything.
target_compile_options(raycaster PRIVATE -Wall -Wextra)
