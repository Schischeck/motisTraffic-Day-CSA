cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-geo_collector-files src/*.cc)

add_library(motis-geo_collector STATIC ${motis-geo_collector-files})
set_target_properties(motis-geo_collector PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-geo_collector
  motis-module
  motis-db
)
