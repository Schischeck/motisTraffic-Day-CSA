project(motis)
cmake_minimum_required(VERSION 2.8)

set(protocol-dir ${CMAKE_CURRENT_SOURCE_DIR}/../../protocol)
file(GLOB_RECURSE protocol-files ${protocol-dir}/*.fbs)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/protocol")
build_flatbuffers(
  32                           # num_bits
  "--everything-required"      # commandline_options
  "${protocol-files}"          # flatbuffers_schemas
  "${protocol-dir}"            # schema_include_dirs
  generated-protocol-headers   # custom_target_name
  ""                           # additional_dependencies
  "${generated-headers-dir}"   # generated_includes_dir
  ""                           # binary_schemas_dir
  ""                           # copy_text_schemas_dir
)

set(resource-header "${generated-headers-dir}/resources.h")
compile_flatbuffers_to_resource_header(
  "${protocol-dir}/Message.fbs"
  "${resource-header}"
  "${protocol-files}"
)

file(GLOB_RECURSE motis-module-files src/*.cc)
add_library(motis-module STATIC ${motis-module-files} ${resource-header})
add_dependencies(motis-module generated-protocol-headers)
target_link_libraries(motis-module flatbuffers32 ctx conf ${Boost_THREAD_LIBRARY})
set_target_properties(motis-module PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_compile_definitions(motis-module PRIVATE FLATBUFFERS_32=1)