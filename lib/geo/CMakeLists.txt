project(motis)
cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE motis-lib-geo-files src/*.cc)
add_library(motis-lib-geo STATIC ${motis-lib-geo-files})
set_target_properties(motis-lib-geo PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})

