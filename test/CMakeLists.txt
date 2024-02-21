#third_party
include_directories(
  ${PROJECT_SOURCE_DIR}/third_party/include/
  ${PROJECT_SOURCE_DIR}/../rtcbase/src/
  ${PROJECT_SOURCE_DIR}/include/
)

link_directories(
  ${PROJECT_SOURCE_DIR}/third_party/lib/
  ${PROJECT_SOURCE_DIR}/../rtcbase/out/
)

file(GLOB_RECURSE all_tests *.cpp)
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM  all_src ${PROJECT_SOURCE_DIR}/src/main.cpp)

foreach(v ${all_tests})
    string(REGEX MATCH "test/.*" relative_path ${v})
    string(REGEX REPLACE "test/" "" target_name ${relative_path})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})

    add_executable(${target_name} ${all_src} ${v})
    target_link_libraries(${target_name} libyaml-cpp.a libev.a libabsl_strings.a libabsl_throw_delegate.a librtcbase.a)
endforeach()
