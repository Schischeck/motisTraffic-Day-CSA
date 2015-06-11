cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-routing-files src/*.cpp)
add_library(motis-routing SHARED ${motis-routing-files})
target_link_libraries(motis-routing ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})
set_target_properties(motis-routing PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
