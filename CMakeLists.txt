cmake_minimum_required(VERSION 2.8)
project(motis)

set(CMAKE_SHARED_LIBRARY_SUFFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS system filesystem regex program_options REQUIRED)
find_package(Threads)

add_library(json11 EXCLUDE_FROM_ALL external_lib/json11/json11.cpp)
set_target_properties(json11 PROPERTIES COMPILE_FLAGS "-std=c++11 -fPIC")

file(GLOB_RECURSE global_headers *.h *.hpp)

MESSAGE(STATUS "boost-heads found at: " ${Boost_INCLUDE_DIR})
MESSAGE(STATUS "boost-libs found at: " ${Boost_LIBRARY_DIRS})

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/external_lib)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/conf/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/net/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/websocketpp)
include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/loader/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/module/include)

add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)

add_subdirectory(motis/loader EXCLUDE_FROM_ALL)
add_subdirectory(motis/module EXCLUDE_FROM_ALL)

add_subdirectory(modules/railviz)
add_subdirectory(modules/routing)
add_subdirectory(modules/reliability)

add_subdirectory(motis/prep)
add_subdirectory(motis/webservice)
