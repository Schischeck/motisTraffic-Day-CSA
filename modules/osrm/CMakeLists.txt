cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-osrm-files src/*.cc)

add_library(motis-osrm STATIC ${motis-osrm-files})
target_link_libraries(motis-osrm
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  osrm
  motis-module
)

set_target_properties(motis-osrm PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
