cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-cli-files src/*.cc)
add_executable(motis-cli EXCLUDE_FROM_ALL ${motis-cli-files})

target_link_libraries(motis-cli
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${NETWORKING}
  snappy
  conf
  motis-module
  net-tcp_client
)

set_target_properties(motis-cli PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-cli PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
