cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(${CMAKE_SORUCE_DIR}/motis/loader/include)
include_directories(include)

file(GLOB_RECURSE motis-loader-files src/*.cpp)
add_library(motis-loader STATIC ${motis-loader-files})
target_link_libraries(motis-loader ${Boost_FILESYSTEM_LIBRARY})
set_target_properties(motis-loader PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")