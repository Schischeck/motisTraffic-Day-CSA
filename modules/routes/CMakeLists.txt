cmake_minimum_required(VERSION 3.0)
project(motis)

set(routes-fbs-dir ${CMAKE_CURRENT_SOURCE_DIR}/fbs)
set(generated-headers-dir "${CMAKE_BINARY_DIR}/generated/motis/routes/fbs")
file(GLOB_RECURSE routes-fbs-files ${routes-fbs-dir}/*.fbs)

build_flatbuffers(
  32
  "--everything-required"        # commandline_options
  "${routes-fbs-files}"          # flatbuffers_schemas
  "${routes-fbs-dir}"            # schema_include_dirs
  generated-routes-fbs-headers   # custom_target_name
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

file(GLOB_RECURSE motis-routes-prepare-files src/prepare.cc src/prepare/*.cc)

file(GLOB_RECURSE motis-routes-files src/*.cc)

list(APPEND motis-routes-prepare-files src/db/rocksdb.cc src/db/db_builder.cc)

add_library(motis-routes STATIC ${motis-routes-files}) 
add_dependencies(motis-routes generated-routes-fbs-headers)
target_link_libraries(motis-routes
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  expat
  rocksdb
  ${ZLIB_LIBRARIES}
  geo
  motis-module
)
set_target_properties(motis-routes PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})

add_executable(routes-prepare EXCLUDE_FROM_ALL ${motis-routes-prepare-files})
add_dependencies(routes-prepare generated-routes-fbs-headers)
target_link_libraries(routes-prepare
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${ZLIB_LIBRARIES}
  expat
  motis-loader
  conf
  geo
  rocksdb
)

set_target_properties(routes-prepare PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(routes-prepare PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
