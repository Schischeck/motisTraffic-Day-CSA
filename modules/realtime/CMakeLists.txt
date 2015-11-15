cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-realtime-files src/*.cc)
add_library(motis-realtime STATIC ${motis-realtime-files})
target_link_libraries(motis-realtime
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    motis-module
    motis-loader
)
set_target_properties(motis-realtime PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})