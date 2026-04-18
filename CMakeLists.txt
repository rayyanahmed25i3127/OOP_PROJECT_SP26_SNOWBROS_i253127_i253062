cmake_minimum_required(VERSION 3.16)
project(snow-bros)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(snow-bros ${SOURCES})
target_include_directories(snow-bros PRIVATE include)
target_link_libraries(snow-bros PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)