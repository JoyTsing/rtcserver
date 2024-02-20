file(GLOB_RECURSE all_src CONFIGURE_DEPENDS *.cpp)

#third_party
include_directories(${PROJECT_SOURCE_DIR}/third_party/include)
link_directories(${PROJECT_SOURCE_DIR}/third_party/lib)

add_executable(xrtcserver ${all_src})

target_link_libraries(xrtcserver libyaml-cpp.a)