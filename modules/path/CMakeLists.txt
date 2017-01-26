cmake_minimum_required(VERSION 3.0)
project(motis)

set(path-fbs-dir ${CMAKE_CURRENT_SOURCE_DIR}/fbs)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/path/fbs")
file(GLOB_RECURSE path-fbs-files ${path-fbs-dir}/*.fbs)

build_flatbuffers(
  32
  "--everything-required"        # commandline_options
  "${path-fbs-files}"          # flatbuffers_schemas
  "${path-fbs-dir}"            # schema_include_dirs
  generated-path-fbs-headers   # custom_target_name
  ""                             # additional_dependencies
  "${generated-headers-dir}"     # generated_includes_dir
  ""                             # binary_schemas_dir
  ""                             # copy_text_schemas_dir
)

include_directories(include)
if (NOT ZLIB_FOUND)
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE motis-path-prepare-files src/prepare.cc src/prepare/*.cc)
file(GLOB_RECURSE motis-path-db-files src/db/*.cc)

file(GLOB_RECURSE motis-path-files src/*.cc)

foreach(file ${motis-path-prepare-files})
  list(REMOVE_ITEM motis-path-files "${file}")
endforeach()

add_library(motis-path STATIC ${motis-path-files})
add_dependencies(motis-path generated-path-fbs-headers)
target_link_libraries(motis-path
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  expat
  rocksdb
  ${ZLIB_LIBRARIES}
  geo
  osrm
  motis-module
)
set_target_properties(motis-path PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})

add_executable(path-prepare EXCLUDE_FROM_ALL ${motis-path-db-files} ${motis-path-prepare-files})
add_dependencies(path-prepare generated-path-fbs-headers)
target_link_libraries(path-prepare
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${ZLIB_LIBRARIES}
  expat
  motis-module
  motis-loader
  conf
  geo
  rocksdb
  osrm
)

set_target_properties(path-prepare PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(path-prepare PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
