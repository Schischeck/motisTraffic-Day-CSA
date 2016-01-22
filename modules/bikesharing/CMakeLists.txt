cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

set(bikesharing-dbschema-dir ${CMAKE_CURRENT_SOURCE_DIR}/dbschema)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/bikesharing/dbschema")
file(GLOB_RECURSE bikesharing-dbschema-files ${bikesharing-dbschema-dir}/*.fbs)

compile_flatbuffers_schema_to_cpp(
  "--everything-required"
  ${bikesharing-dbschema-dir}
  ${generated-headers-dir}
  generated-headers
  ${bikesharing-dbschema-files}
)

file(GLOB_RECURSE motis-bikesharing-files src/*.cc)
add_library(motis-bikesharing STATIC 
  ${motis-bikesharing-files}
  ${generated-headers}
)
set_target_properties(motis-bikesharing PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-bikesharing 
  motis-module 
  pugixml
  leveldb
)
