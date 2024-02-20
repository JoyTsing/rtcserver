cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif()

add_definitions("-g -pipe -W -Wall -fPIC -std=gnu++14 -DWEBRTC_POSIX
-DWEBRTC_LINUX")

PROJECT(rtcserver LANGUAGES CXX)

file(GLOB add_src "./src/*.cpp")

add_executable(rtcserver ${add_src})