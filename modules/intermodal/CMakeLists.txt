cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-intermodal-files src/*.cc)
add_library(motis-intermodal STATIC ${motis-intermodal-files})
target_link_libraries(motis-intermodal
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  motis-module
  motis-core
  geo
)

set_target_properties(motis-intermodal PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
