cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)
include_directories(../routing/include)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
find_package(MySQL REQUIRED)
find_package(MySQLpp REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR} ${MYSQLPP_INCLUDE_DIR})

file(GLOB_RECURSE motis-realtime-files src/*.cc)
add_library(motis-realtime SHARED ${motis-realtime-files})
target_link_libraries(motis-realtime ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_link_libraries(motis-realtime ${MYSQL_LIBRARIES} ${MYSQLPP_LIBRARIES})
target_link_libraries(motis-realtime motis-loader)
set_target_properties(motis-realtime PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-realtime PROPERTIES OUTPUT_NAME realtime.module)

add_custom_target(motis-realtime-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-realtime-reload motis-realtime motis-webservice)

file(GLOB_RECURSE motis-realtime-test-files test/*.cc)
add_executable(motis-realtime-test ${motis-realtime-test-files})
set_target_properties(motis-realtime-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-realtime-test motis-realtime motis-loader motis-routing)
