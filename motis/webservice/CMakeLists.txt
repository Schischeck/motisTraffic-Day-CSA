cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-webservice-files src/*.cc)
add_executable(motis-webservice ${motis-webservice-files})
target_link_libraries(motis-webservice ${CMAKE_THREAD_LIBS_INIT} motis-loader motis-module conf json11)
if (NOT MSVC)
  set_target_properties(motis-webservice PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()
if (MSVC)
  set_target_properties(motis-webservice PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_NOEXCEPT_)
endif()
