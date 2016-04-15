cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-db-files src/*.cc)
add_library(motis-db STATIC ${motis-db-files})

set_target_properties(motis-db PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-db pq)
