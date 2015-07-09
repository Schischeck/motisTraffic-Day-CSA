cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-guesser-files src/*.cc)
add_library(motis-guesser SHARED ${motis-guesser-files})
target_link_libraries(motis-guesser ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} guess motis-module)
set_target_properties(motis-guesser PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-guesser PROPERTIES OUTPUT_NAME guesser.module)

add_custom_target(motis-guesser-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-guesser-reload motis-guesser motis-webservice)
