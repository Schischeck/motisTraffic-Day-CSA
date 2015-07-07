cmake_minimum_required(VERSION 2.8)
project(motis)

set(CMAKE_SHARED_LIBRARY_SUFFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem regex program_options REQUIRED)
find_package(Threads)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/websocketpp)
include_directories(${CMAKE_SOURCE_DIR}/external_lib)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/flatbuffers/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/conf/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/net/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/loader/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/module/include)
include_directories(${CMAKE_BINARY_DIR}/generated)

include(${CMAKE_SOURCE_DIR}/cmake/FlatBuffersUtil.cmake)

add_subdirectory(external_lib/flatbuffers EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)

add_subdirectory(motis/core EXCLUDE_FROM_ALL)
add_subdirectory(motis/loader EXCLUDE_FROM_ALL)
add_subdirectory(motis/module EXCLUDE_FROM_ALL)

add_subdirectory(modules/railviz)
add_subdirectory(modules/routing)
add_subdirectory(modules/reliability)
add_subdirectory(modules/guesser)

add_subdirectory(motis/prep)
add_subdirectory(motis/webservice)