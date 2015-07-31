cmake_minimum_required(VERSION 2.8)
project(motis)

find_package(Boost REQUIRED COMPONENTS system filesystem regex)

add_library(json11 EXCLUDE_FROM_ALL external_lib/json11/json11.cpp)
set_target_properties(json11 PROPERTIES COMPILE_FLAGS -std=c++11)

add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)

add_subdirectory(motis/loader EXCLUDE_FROM_ALL)
add_subdirectory(motis/prep EXCLUDE_FROM_ALL)
add_subdirectory(motis/webservice EXCLUDE_FROM_ALL)

add_subdirectory(modules/railviz EXCLUDE_FROM_ALL)
add_subdirectory(modules/routing EXCLUDE_FROM_ALL)