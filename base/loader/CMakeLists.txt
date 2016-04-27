cmake_minimum_required(VERSION 2.8)
project(motis)

set(schedule-dir ${CMAKE_CURRENT_SOURCE_DIR}/schedule-format)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/schedule-format")
file(GLOB_RECURSE schedule-format-files ${schedule-dir}/*.fbs)
build_flatbuffers(
  ""                           # commandline_options
  "${schedule-format-files}"   # flatbuffers_schemas
  "${schedule-dir}"            # schema_include_dirs
  generated-schedule-headers   # custom_target_name
  ""                           # additional_dependencies
  "${generated-headers-dir}"   # generated_includes_dir
  ""                           # binary_schemas_dir
  ""                           # copy_text_schemas_dir
)

file(GLOB_RECURSE motis-loader-files src/*.cc)
add_library(motis-loader STATIC ${motis-loader-files})
add_dependencies(motis-loader generated-schedule-headers)
target_link_libraries(motis-loader ${Boost_FILESYSTEM_LIBRARY})
set_target_properties(motis-loader PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
