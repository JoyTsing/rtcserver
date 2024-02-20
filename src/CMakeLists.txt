file(GLOB_RECURSE all_src CONFIGURE_DEPENDS *.cpp)

#third_party
include_directories(
  ${PROJECT_SOURCE_DIR}/third_party/include
  ${PROJECT_SOURCE_DIR}/third_party/rtcbase/src
)


link_directories(
  ${PROJECT_SOURCE_DIR}/third_party/lib
  ${PROJECT_SOURCE_DIR}/third_party/rtcbase/out
)
add_executable(xrtcserver ${all_src})

target_link_libraries(xrtcserver libyaml-cpp.a libabsl_strings.a libabsl_throw_delegate.a)