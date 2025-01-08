cmake_minimum_required(VERSION 3.10)

# Build Type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug build.")
    add_compile_options(-std=gnu++17 -g -Wall -Wextra -Wpedantic -fsanitize=thread)
    add_link_options(-fsanitize=thread)
else(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Release build.")
    add_compile_options(-std=gnu++17 -Wall -Wextra -O2 -Wpedantic)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(chat)

# Source files
file(GLOB_RECURSE SOURCES_CLIENT
    src/chat/*.cpp
    src/common/*.cpp
)

file(GLOB_RECURSE SOURCES_SERVER
    src/serveur/*.cpp
    src/common/*.cpp
)

# Binaries
add_executable(chat ${SOURCES_CLIENT})

add_executable(serveur-chat ${SOURCES_SERVER})
