project(motis)
cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR}/external_lib)
include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/module/include)
include_directories(include)

file(GLOB_RECURSE motis-railviz-files src/*.cc)
add_library(motis-railviz ${motis-railviz-files})
set_target_properties(motis-railviz PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)