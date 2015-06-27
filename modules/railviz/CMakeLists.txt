cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-railviz-files src/*.cc)
add_library(motis-railviz SHARED ${motis-railviz-files} ${global_headers})
MESSAGE(STATUS "boost-system-library: " ${Boost_SYSTEM_LIBRARY})
target_link_libraries(motis-railviz ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} json11)
set_target_properties(motis-railviz PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
set_target_properties(motis-railviz PROPERTIES OUTPUT_NAME railviz.module)

add_custom_target(motis-railviz-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-railviz-reload motis-railviz motis-webservice)

add_definitions(-DBOOST_TEST_DYN_LINK)
file(GLOB_RECURSE motis-railviz-test-files test/*.cc)
add_executable(motis-railviz-test ${motis-railviz-files} ${motis-railviz-test-files})
set_target_properties(motis-railviz-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-railviz-test ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} json11 motis-loader ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
