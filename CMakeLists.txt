cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif()

add_definitions("-g -pipe -W -Wall -fPIC -std=gnu++14 -DWEBRTC_POSIX
-DWEBRTC_LINUX")

PROJECT(rtcserver LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(./third_party)

add_subdirectory(./src)
