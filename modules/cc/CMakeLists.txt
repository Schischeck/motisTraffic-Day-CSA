cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-cc-files src/*.cc)
add_library(motis-cc STATIC ${motis-cc-files})
target_link_libraries(motis-cc ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} motis-module)
set_target_properties(motis-cc PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
