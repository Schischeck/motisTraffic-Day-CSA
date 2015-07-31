cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(${CMAKE_SOURCE_DIR}/external_lib/conf/include)

include_directories(${CMAKE_SOURCE_DIR}/motis/loader/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(include)

file(GLOB_RECURSE motis-prep-files src/*.cpp)
add_executable(motis-prep ${motis-prep-files})
target_link_libraries(motis-prep conf motis-loader)
set_target_properties(motis-prep PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")