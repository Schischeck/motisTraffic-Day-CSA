cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-user-files src/*.cc)

add_library(motis-user STATIC ${motis-user-files})
set_target_properties(motis-user PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-user motis-module pq)
