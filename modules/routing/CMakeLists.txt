cmake_minimum_required(VERSION 3.0)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-routing-files src/*.cc)
add_library(motis-routing STATIC ${motis-routing-files})
target_link_libraries(motis-routing
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  motis-module
  motis-core
)
set_target_properties(motis-routing PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})

include_directories(eval/include)

file(GLOB_RECURSE motis-generator-files eval/src/generator/*.cc)
add_executable(motis-generator EXCLUDE_FROM_ALL ${motis-generator-files})
target_link_libraries(motis-generator motis-bootstrap motis-core conf ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(motis-generator PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-generator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

file(GLOB_RECURSE motis-analyzer-files eval/src/analyzer/*.cc)
add_executable(motis-analyzer EXCLUDE_FROM_ALL ${motis-analyzer-files})
target_link_libraries(motis-analyzer conf motis-module motis-core ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(motis-analyzer PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-analyzer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

file(GLOB_RECURSE motis-comparator-files eval/src/comparator/*.cc)
add_executable(motis-comparator EXCLUDE_FROM_ALL ${motis-comparator-files})
target_link_libraries(motis-comparator conf motis-module motis-core ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(motis-comparator PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(motis-comparator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_custom_target(motis-eval)
add_dependencies(motis-eval motis-generator motis-analyzer motis-comparator)
