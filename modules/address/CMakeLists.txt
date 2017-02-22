cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-address-files src/*.cc)
add_library(motis-address STATIC ${motis-address-files})
target_link_libraries(motis-address
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  address-typeahead
  motis-module
  motis-core
)
set_target_properties(motis-address PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
