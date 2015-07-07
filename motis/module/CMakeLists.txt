project(motis)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE motis-module-files src/*.cc)
add_library(motis-module STATIC ${motis-module-files})
target_link_libraries(motis-module ${CMAKE_DL_LIBS})
set_target_properties(motis-module PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
