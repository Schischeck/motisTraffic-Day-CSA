cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)
include_directories(../routing/include)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")

file(GLOB_RECURSE motis-realtime-files src/*.cc)
add_library(motis-realtime STATIC ${motis-realtime-files})
target_link_libraries(motis-realtime 
    ${Boost_PROGRAM_OPTIONS_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY}
    motis-module
    motis-loader
)
set_target_properties(motis-realtime PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)

file(GLOB_RECURSE motis-realtime-test-files test/*.cc)
add_executable(motis-realtime-test EXCLUDE_FROM_ALL ${motis-realtime-test-files})
set_target_properties(motis-realtime-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-realtime-test motis-realtime motis-loader motis-routing)
