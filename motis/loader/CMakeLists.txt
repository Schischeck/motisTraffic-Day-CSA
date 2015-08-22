cmake_minimum_required(VERSION 2.8)
project(motis)

set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/schedule-format")
file(GLOB_RECURSE schedule-format-files ${CMAKE_CURRENT_SOURCE_DIR}/schedule-format/*.fbs)
compile_flatbuffers_schema_to_cpp(${generated-headers-dir} generated-headers ${schedule-format-files})

set(generated-converter-header-dir "${CMAKE_BINARY_DIR}/generated/motis/converter")
file(GLOB converter-files ${CMAKE_CURRENT_SOURCE_DIR}/test/Converter.fbs)
compile_flatbuffers_schema_to_cpp(${generated-converter-header-dir} converter-headers ${converter-files})

file(GLOB_RECURSE motis-loader-files src/*.cc)
add_library(motis-loader STATIC ${motis-loader-files} ${generated-headers})
target_link_libraries(motis-loader ${Boost_FILESYSTEM_LIBRARY})
if (NOT MSVC)
  set_target_properties(motis-loader PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()

file(GLOB_RECURSE motis-loader-test-files test/*.cc)
add_executable(motis-loader-test EXCLUDE_FROM_ALL ${motis-loader-test-files} ${converter-headers})
set_target_properties(motis-loader-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-loader-test ${Boost_SYSTEM_LIBRARY} motis-loader)
