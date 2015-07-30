cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-prep-files src/*.cc)
add_executable(motis-prep ${motis-prep-files})
target_link_libraries(motis-prep motis-loader conf)
if (NOT MSVC)
  set_target_properties(motis-prep PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()
