project(motis)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE motis-core-test-files test/*.cc)
add_executable(motis-core-test ${motis-core-test-files})
set_target_properties(motis-core-test PROPERTIES COMPILE_FLAGS -std=c++11)
target_link_libraries(motis-core-test motis-loader)
