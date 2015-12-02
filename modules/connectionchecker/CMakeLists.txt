cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-connectionchecker-files src/*.cc)
add_library(motis-connectionchecker STATIC ${motis-connectionchecker-files})
set_target_properties(motis-connectionchecker PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-connectionchecker 
  motis-module
  motis-core
)
