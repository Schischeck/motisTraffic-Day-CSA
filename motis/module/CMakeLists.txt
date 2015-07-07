project(motis)
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_SOURCE_DIR}/cmake/FlatBuffersUtil.cmake)

file(GLOB_RECURSE motis-module-files src/*.cc)
add_library(motis-module STATIC ${motis-module-files} ${generated-headers} ${resource-header})
target_link_libraries(motis-module flatbuffers-gen-text ${CMAKE_DL_LIBS})
set_target_properties(motis-module PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
