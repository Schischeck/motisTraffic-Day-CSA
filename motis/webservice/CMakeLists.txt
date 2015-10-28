cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/modules/guesser/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/railviz/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/realtime/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/reliability/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/ris/include)

file(GLOB_RECURSE motis-webservice-files src/*.cc)
add_executable(motis-webservice ${motis-webservice-files})

target_link_libraries(motis-webservice
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
  motis-ris
)

set_target_properties(motis-webservice PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_CPP11_STL_)
if (NOT MSVC)
  set_target_properties(motis-webservice PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()
