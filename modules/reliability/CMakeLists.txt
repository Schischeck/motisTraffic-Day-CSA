cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include test)

file(GLOB_RECURSE motis-reliability-files src/*.cc)
add_library(motis-reliability STATIC ${motis-reliability-files})
target_link_libraries(motis-reliability ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} motis-module motis-core)
set_target_properties(motis-reliability PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})