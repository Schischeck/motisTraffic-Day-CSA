cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-generator-files src/generator/*.cc)
add_executable(motis-generator EXCLUDE_FROM_ALL ${motis-generator-files})
target_link_libraries(motis-generator ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} motis-bootstrap conf)
set_target_properties(motis-generator PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-generator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

file(GLOB_RECURSE motis-analyzer-files src/analyzer/*.cc)
add_executable(motis-analyzer EXCLUDE_FROM_ALL ${motis-analyzer-files})
target_link_libraries(motis-analyzer ${Boost_SYSTEM_LIBRARY} conf)
set_target_properties(motis-analyzer PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-analyzer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

file(GLOB_RECURSE motis-comparator-files src/comparator/*.cc)
add_executable(motis-comparator EXCLUDE_FROM_ALL ${motis-comparator-files})
target_link_libraries(motis-comparator ${Boost_SYSTEM_LIBRARY} conf motis-module)
set_target_properties(motis-comparator PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-comparator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_custom_target(motis-eval)
add_dependencies(motis-eval motis-generator motis-analyzer motis-comparator)