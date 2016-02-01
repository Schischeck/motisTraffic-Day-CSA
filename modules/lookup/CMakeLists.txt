cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-lookup-files src/*.cc)
add_library(motis-lookup STATIC ${motis-lookup-files})
set_target_properties(motis-lookup PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-lookup 
  ${Boost_PROGRAM_OPTIONS_LIBRARY} 
  ${Boost_SYSTEM_LIBRARY} 
  motis-module
)