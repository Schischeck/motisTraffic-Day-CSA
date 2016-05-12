cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-reliability-files src/*.cc)
add_library(motis-reliability STATIC ${motis-reliability-files})
set_target_properties(motis-reliability PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-reliability 
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  motis-module
  motis-core
)
