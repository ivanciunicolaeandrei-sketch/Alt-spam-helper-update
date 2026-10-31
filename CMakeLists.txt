cmake_minimum_required(VERSION 3.21)
project(AltSpamHelper VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(${PROJECT_NAME} SHARED src/main.cpp)
if(NOT DEFINED ENV{GEODE_SDK})
  message(FATAL_ERROR "GEODE_SDK is not set")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# GDReplayFormat GDR1: supports legacy .gdr (MessagePack) and .gdr.json.
CPMAddPackage("gh:maxnut/GDReplayFormat#77bd505853541a87a12895dc23de05f6585d0515")
target_link_libraries(${PROJECT_NAME} libGDR)

setup_geode_mod(${PROJECT_NAME})
