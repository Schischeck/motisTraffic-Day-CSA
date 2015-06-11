cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-railviz-files src/*.cc)
add_library(motis-railviz SHARED ${motis-railviz-files})
target_link_libraries(motis-railviz ${Boost_SYSTEM_LIBRARY} json11)
set_target_properties(motis-railviz PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-railviz PROPERTIES OUTPUT_NAME railviz.module)

add_custom_target(motis-railviz-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-railviz-reload motis-railviz motis-webservice)