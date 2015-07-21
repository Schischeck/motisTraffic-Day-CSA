cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include test)

file(GLOB_RECURSE motis-reliability-files src/*.cc)
add_library(motis-reliability SHARED ${motis-reliability-files})
target_link_libraries(motis-reliability ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} motis-module)
set_target_properties(motis-reliability PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-reliability PROPERTIES OUTPUT_NAME reliability.module)

add_custom_target(motis-reliability-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-reliability-reload motis-reliability)

file(GLOB_RECURSE motis-reliability-test-files test/*.cc)
add_executable(motis-reliability-test ${motis-reliability-files} ${motis-reliability-test-files})
set_target_properties(motis-reliability-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-reliability-test ${Boost_SYSTEM_LIBRARY} motis-loader)
