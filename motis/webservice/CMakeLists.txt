cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-webservice-files src/*.cc)
add_executable(motis-webservice ${motis-webservice-files})
if (MOTIS_STATIC_MODULES)
  target_link_libraries(motis-webservice
    ${CMAKE_THREAD_LIBS_INIT}
    conf
    motis-loader
    motis-module
    motis-guesser
    motis-railviz
    motis-reliability
    motis-routing
  )
else()
  target_link_libraries(motis-webservice ${CMAKE_THREAD_LIBS_INIT} motis-loader motis-module conf)
endif()
set_target_properties(motis-webservice PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_CPP11_STL_)
if (NOT MSVC)
  set_target_properties(motis-webservice PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()
