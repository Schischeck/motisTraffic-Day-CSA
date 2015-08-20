cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

# prepare all test-

file(GLOB_RECURSE motis-railviz-files src/*.cc)
if (MOTIS_STATIC_MODULES)
  add_library(motis-railviz STATIC ${motis-railviz-files})
else()
  add_library(motis-railviz SHARED ${motis-railviz-files})
endif()
target_link_libraries(motis-railviz ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} motis-module)
set_target_properties(motis-railviz PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
set_target_properties(motis-railviz PROPERTIES OUTPUT_NAME railviz.module)

add_custom_target(motis-railviz-reload killall -SIGUSR1 motis-webservice)
add_dependencies(motis-railviz-reload motis-railviz motis-webservice)

file(GLOB_RECURSE motis-railviz-test-files test/*.cc)
add_executable(motis-railviz-test ${motis-railviz-files} ${motis-railviz-test-files})
set_target_properties(motis-railviz-test PROPERTIES COMPILE_FLAGS -std=c++11 -Wall -Wextra)
target_link_libraries(motis-railviz-test ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} motis-loader motis-module)
