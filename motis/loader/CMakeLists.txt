cmake_minimum_required(VERSION 2.8)
project(motis)

file(GLOB_RECURSE motis-loader-files src/*.cpp)
add_library(motis-loader STATIC ${motis-loader-files})
target_link_libraries(motis-loader ${Boost_FILESYSTEM_LIBRARY})
set_target_properties(motis-loader PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
