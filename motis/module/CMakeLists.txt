project(motis)
cmake_minimum_required(VERSION 2.8)

set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/protocol")
set(resource-header "${generated-headers-dir}/resources.h")
compile_flatbuffers_to_resource_header("${CMAKE_SOURCE_DIR}/protocol/Message.fbs" "${resource-header}")

set(protocol-dir ${CMAKE_SOURCE_DIR}/protocol)
file(GLOB_RECURSE protocol-files ${protocol-dir}/*.fbs)
compile_flatbuffers_schema_to_cpp("--everything-required" ${protocol-dir} ${generated-headers-dir} generated-headers ${protocol-files})

file(GLOB_RECURSE motis-module-files src/*.cc)
add_library(motis-module STATIC ${motis-module-files} ${generated-headers} ${resource-header})
target_link_libraries(motis-module flatbuffers-gen-text ${Boost_THREAD})
set_target_properties(motis-module PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
