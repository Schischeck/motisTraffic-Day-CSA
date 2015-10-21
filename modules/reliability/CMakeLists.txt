cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include test)

file(GLOB_RECURSE motis-reliability-files src/*.cc)
add_library(motis-reliability STATIC ${motis-reliability-files})
target_link_libraries(motis-reliability ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} motis-module motis-core)
set_target_properties(motis-reliability PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)

file(GLOB_RECURSE motis-reliability-test-files test/*.cc)
add_executable(motis-reliability-test EXCLUDE_FROM_ALL ${motis-reliability-test-files})
set_target_properties(motis-reliability-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-reliability-test ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} motis-loader motis-routing motis-reliability)

