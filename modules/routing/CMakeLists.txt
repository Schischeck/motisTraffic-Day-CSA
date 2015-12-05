cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-routing-files src/*.cc)
add_library(motis-routing STATIC ${motis-routing-files})
target_link_libraries(motis-routing
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  motis-module
  motis-core
)
set_target_properties(motis-routing PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
