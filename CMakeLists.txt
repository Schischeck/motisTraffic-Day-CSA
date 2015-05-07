cmake_minimum_required(VERSION 2.6)
project(td)

foreach(flags CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

foreach(flags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

add_definitions(-DROUTE_EDGE_SEARCH_TRACE)

if (MSVC)
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${BOOST_ROOT}\\stage\\lib")
  link_directories(${CMAKE_LIBRARY_PATH})
endif()

################################
# Configure ccache
################################
option(USE_CCACHE "USE_CCACHE" OFF)
if (USE_CCACHE)
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()


################################
# Dependencies
################################
find_package(Threads)
find_package(Boost COMPONENTS system regex filesystem)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/pugixml-1.2/scripts EXCLUDE_FROM_ALL)

#//TODO check websocketpp library

include_directories(src)
include_directories(external_lib/pugixml-1.2/src)
include_directories(external_lib/net/include)
include_directories(external_lib/conf/include)
include_directories(external_lib/rapidjson/include)
include_directories(${Boost_INCLUDE_DIR})


################################
# Source File Groups
################################
file(GLOB_RECURSE TD_BASE_FILES "src/*.cpp")
file(GLOB_RECURSE TD_EXE_FILES "src/execution/*.cpp")
file(GLOB_RECURSE TD_SERIALIZATION "src/serialization/*.cpp")
file(GLOB TD_BATCH_FILES "src/execution/batch.cpp")
file(GLOB TD_DESERIALIZER "src/serialization/Deserializer.cpp")
list(APPEND TD_BASE_FILES ${TD_DESERIALIZER})
list(REMOVE_ITEM TD_SERIALIZATION ${TD_DESERIALIZER})
list(REMOVE_ITEM TD_BASE_FILES ${TD_EXE_FILES})
list(REMOVE_ITEM TD_BASE_FILES ${TD_SERIALIZATION})
list(REMOVE_ITEM TD_EXE_FILES  ${TD_BATCH_FILES})


################################
# Library
################################
add_library(td-base STATIC ${TD_BASE_FILES})
target_link_libraries(td-base ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} pugixml)
if (NOT MSVC)
  set_target_properties(td-base PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()


################################
# Service Executable
################################
add_executable(td ${TD_EXE_FILES})
target_link_libraries(td td-base conf net-http_server net-tcp_client)
if(MSVC)
  set(_def
    _VARIADIC_MAX=10
    _WIN32_WINNT=0x0501
  )
  set_target_properties(td PROPERTIES COMPILE_DEFINITIONS "${_def}")
elseif(MINGW)
  set_target_properties(td PROPERTIES COMPILE_FLAGS "-std=c++11")
  set(_def
    BOOST_THREAD_USE_LIB
    WINVER=0x0501
    _WIN32_WINNT=0x0501
    _WIN32_IE=0x0501
  )
  set_target_properties(td PROPERTIES COMPILE_DEFINITIONS "${_def}")
else()
  set_target_properties(td PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()


################################
# Batch Executable
################################
add_executable(td-batch
  ${TD_BATCH_FILES}
  "src/execution/dataset_settings.cpp"
  "src/execution/query_file_settings.cpp"
  "src/execution/parallel_settings.cpp"
)
target_link_libraries(td-batch td-base conf)
if (NOT MSVC)
  set_target_properties(td-batch PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()


################################
# Serialization
################################
add_executable(serialize ${TD_SERIALIZATION})
target_link_libraries(serialize td-base conf)
if (NOT MSVC)
  set_target_properties(serialize PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()

################################
# Tests
################################
if (NOT MSVC AND NOT MINGW)
  if (NOT TARGET gtest)
    add_subdirectory(test/gtest-1.7.0 EXCLUDE_FROM_ALL)
  endif()

  enable_testing()
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  file(GLOB test_files "test/*.cpp")
  add_executable(td-tests EXCLUDE_FROM_ALL ${test_files})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(td-tests gtest gtest_main td-base)
  else()
    target_link_libraries(td-tests gtest gtest_main td-base -pthread)
  endif()
  set_target_properties(td-tests PROPERTIES COMPILE_FLAGS "-std=c++11")

  add_test(td-tests td-tests)
endif()
