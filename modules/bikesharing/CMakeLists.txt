cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-bikesharing-files src/*.cc)
add_library(motis-bikesharing STATIC ${motis-bikesharing-files})
set_target_properties(motis-bikesharing PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-bikesharing 
  motis-module 
  pugixml
)
