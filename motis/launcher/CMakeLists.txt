cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-launcher-files src/*.cc)
add_executable(motis ${motis-launcher-files})

target_link_libraries(motis
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${NETWORKING}
  conf
  motis-loader
  motis-module
  motis-guesser
  motis-railviz
  motis-realtime
  motis-reliability
  motis-routing
  motis-realtime
  motis-ris
)

set_target_properties(motis PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_CPP11_STL_)
set_target_properties(motis PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})