cmake_minimum_required(VERSION 2.8)
project(motis)

file(GLOB_RECURSE global_headers *.h *.hpp)

foreach(flags CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

foreach(flags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(MOTIS_CXX_FLAGS "-std=c++11 -Wall -Wextra")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(MOTIS_CXX_FLAGS "${MOTIS_CXX_FLAGS} -Wno-maybe-uninitialized")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_GENERATOR STREQUAL "Ninja")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

if(MINGW)
  add_definitions(-DWINVER=0x0501)
  add_definitions(-D_WIN32_WINNT=0x0501)
  add_definitions(-D_WIN32_IE=0x0501)
  add_definitions(-D_WEBSOCKETPP_NO_CPP11_THREAD_)
  add_definitions(-DBOOST_THREAD_USE_LIB)
  set(NETWORKING ws2_32 mswsock)
endif()

if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS system filesystem regex program_options thread REQUIRED)
find_package(Threads)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/flatbuffers/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/googletest/googletest/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/libzip/lib)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/pugixml/src)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/sparsehash/src)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/sqlite/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/sqlpp11/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/sqlpp11-connector-sqlite3/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external_lib/websocketpp)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/conf/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/guess/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/net/include)
include_directories(${CMAKE_SOURCE_DIR}/external_lib/parser/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/core/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/loader/include)
include_directories(${CMAKE_SOURCE_DIR}/motis/module/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/guesser/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/railviz/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/realtime/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/reliability/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/ris/include)
include_directories(${CMAKE_SOURCE_DIR}/modules/routing/include)
include_directories(${CMAKE_BINARY_DIR}/generated)

add_subdirectory(external_lib/flatbuffers EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/googletest/googletest EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/pugixml/scripts EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/sqlite EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/sqlpp11-connector-sqlite3/src EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/libzip EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/parser EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/guess EXCLUDE_FROM_ALL)

add_subdirectory(motis/core EXCLUDE_FROM_ALL)
add_subdirectory(motis/loader EXCLUDE_FROM_ALL)
add_subdirectory(motis/module EXCLUDE_FROM_ALL)

function(add_modules)
  foreach(module ${ARGV})
    add_subdirectory(modules/${module})
  endforeach(module)
endfunction()
add_modules(guesser railviz realtime reliability ris routing)

add_subdirectory(motis/launcher)

configure_file (
  ${CMAKE_SOURCE_DIR}/test/test_dir.h.in 
  ${CMAKE_BINARY_DIR}/generated/test_dir.h
)

file(GLOB_RECURSE motis-modules-test-files modules/*_test.cc)
file(GLOB_RECURSE motis-central-test-files motis/*_test.cc)
add_executable(motis-test EXCLUDE_FROM_ALL
  ${CMAKE_SOURCE_DIR}/test/test_main.cc
  ${motis-modules-test-files}
  ${motis-central-test-files})
set_target_properties(motis-test PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-test
  motis-guesser
  motis-railviz
  motis-realtime
  motis-reliability
  motis-ris
  motis-routing
  motis-module
  motis-loader
  ${Boost_LIBRARIES}
)
target_link_libraries(motis-test gtest gtest_main)
