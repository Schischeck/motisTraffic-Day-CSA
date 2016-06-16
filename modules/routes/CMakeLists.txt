cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)
if (NOT ZLIB_FOUND)
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE motis-routes-files src/*.cc)
add_library(motis-routes STATIC ${motis-routes-files})
target_link_libraries(motis-routes ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} expat ${ZLIB_LIBRARIES} motis-module)
set_target_properties(motis-routes PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})


include_directories(include/preprocessing)

file(GLOB_RECURSE routes-preprocessor-files src/preprocessing/*.cc)
add_executable(routes-preprocessor EXCLUDE_FROM_ALL ${routes-preprocessor-files})
target_link_libraries(routes-preprocessor ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${ZLIB_LIBRARIES} expat motis-bootstrap)
set_target_properties(routes-preprocessor PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
set_target_properties(routes-preprocessor PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")