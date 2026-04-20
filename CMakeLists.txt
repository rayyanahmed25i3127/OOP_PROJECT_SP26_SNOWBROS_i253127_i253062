cmake_minimum_required(VERSION 3.16)
project(snow-bros)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# GLOB_RECURSE scans src/ AND all subfolders (src/states/, src/entities/, etc.)
# CONFIGURE_DEPENDS tells CMake to re-check for new files when we rebuild,
# so we don't have to manually re-run cmake every time we add a .cpp file.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(snow-bros ${SOURCES})

# PRIVATE means only this target uses these includes, not anything linking to us.
# This lets us #include "Game.hpp" or "states/GameState.hpp" without relative paths.
target_include_directories(snow-bros PRIVATE include)

target_link_libraries(snow-bros PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)