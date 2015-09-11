cmake_minimum_required(VERSION 2.8)
project(motis)

set(schedule-dir ${CMAKE_CURRENT_SOURCE_DIR}/schedule-format)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/schedule-format")
file(GLOB_RECURSE schedule-format-files ${schedule-dir}/*.fbs)
compile_flatbuffers_schema_to_cpp(${generated-headers-dir} generated-headers ${schedule-format-files})

file(GLOB_RECURSE motis-loader-files src/*.cc)
add_library(motis-loader STATIC ${motis-loader-files} ${generated-headers})
target_link_libraries(motis-loader ${Boost_FILESYSTEM_LIBRARY})
if (NOT MSVC)
  set_target_properties(motis-loader PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra -fPIC")
endif()
