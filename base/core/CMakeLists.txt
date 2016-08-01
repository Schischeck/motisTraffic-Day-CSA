project(motis)
cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE motis-core-files src/*.cc)
add_library(motis-core STATIC ${motis-core-files})
set_target_properties(motis-core PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-core motis-loader motis-module)
