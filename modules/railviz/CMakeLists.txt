cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-railviz-files src/*.cc)
add_library(motis-railviz STATIC ${motis-railviz-files})
target_link_libraries(motis-railviz ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} motis-module)
set_target_properties(motis-railviz PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)