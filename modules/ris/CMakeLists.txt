cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-ris-files src/*.cc)
file(GLOB_RECURSE ris_loader-files src/ris_loader.cc)

list(REMOVE_ITEM motis-ris-files ${ris_loader-files})

add_library(motis-ris STATIC ${motis-ris-files})
set_target_properties(motis-ris PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-ris
  conf
  motis-module
  snappy
  pugixml
  sqlpp11-connector-sqlite3
  sqlite
)

add_executable(ris_loader EXCLUDE_FROM_ALL ${ris_loader-files})
target_link_libraries(ris_loader
  conf
  motis-ris
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY}
)
set_target_properties(ris_loader PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(ris_loader PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
