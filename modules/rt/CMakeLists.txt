cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-rt-files src/*.cc)
add_library(motis-rt STATIC ${motis-rt-files})
target_link_libraries(motis-rt ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} motis-module)
set_target_properties(motis-rt PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})