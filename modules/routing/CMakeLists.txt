cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-routing-files src/*.cc)
add_library(motis-routing SHARED ${motis-routing-files})
target_link_libraries(motis-routing ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} json11)
set_target_properties(motis-routing PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-routing PROPERTIES OUTPUT_NAME routing.module)

add_custom_target(motis-routing-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-routing-reload motis-routing motis-webservice)
