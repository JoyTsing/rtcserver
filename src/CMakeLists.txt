#third_party
include_directories(
  ${PROJECT_SOURCE_DIR}/third_party/include/
  ${PROJECT_SOURCE_DIR}/../rtcbase/src/
)

link_directories(
  ${PROJECT_SOURCE_DIR}/third_party/lib/
  ${PROJECT_SOURCE_DIR}/../rtcbase/out/
)

file(GLOB_RECURSE all_src CONFIGURE_DEPENDS *.cpp)

add_executable(rtcserver ${all_src})

target_link_libraries(rtcserver libjsoncpp.a libyaml-cpp.a libev.a libabsl_strings.a libabsl_throw_delegate.a librtcbase.a )