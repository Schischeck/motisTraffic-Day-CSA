cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-launcher-files src/*.cc)
add_executable(motis ${motis-launcher-files})

target_link_libraries(motis
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${NETWORKING}
  zstd
  conf
  motis-bootstrap
  net-http_server
)

set_target_properties(motis PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_CPP11_STL_)
set_target_properties(motis PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
