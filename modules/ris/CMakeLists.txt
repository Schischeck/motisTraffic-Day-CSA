cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

file(GLOB_RECURSE motis-ris-files src/*.cc)
add_library(motis-ris STATIC ${motis-ris-files})
set_target_properties(motis-ris PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-ris
    motis-module
    pugixml
    zip
    sqlpp11-connector-sqlite3
    sqlite
)

file(GLOB_RECURSE motis-ris-test-files test/*.cc)
add_executable(motis-ris-test EXCLUDE_FROM_ALL ${motis-ris-test-files})
set_target_properties(motis-ris-test PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-ris-test
    motis-ris
    gtest
    gtest_main
)
