cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-ris-files src/*.cc)
add_library(motis-ris STATIC ${motis-ris-files})
target_link_libraries(motis-ris
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM}
  motis-module
  motis-core
  pugixml
  lmdb
  tar
)
set_target_properties(motis-ris PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
