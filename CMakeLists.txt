set(CMAKE_USER_MAKE_RULES_OVERRIDE
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

cmake_minimum_required(VERSION 3.0)
project(motis)

# Quickfix for GCC 7
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-Wno-implicit-fallthrough)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

# Quickfix for MSVC
if (MSVC)
  add_definitions(/std:c++latest)
  add_definitions(/D_HAS_AUTO_PTR_ETC)
endif()

option(MOTIS_LINT "enable lint (clang-tidy) target" OFF)
option(MOTIS_COV "enable coverage (coverage) target" OFF)

if (MOTIS_LINT)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

include_directories(${CMAKE_BINARY_DIR}/generated)

set(USE_OPENMP OFF CACHE BOOL openmp FORCE)


################################
# MOTIS Version
################################
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE motis_git_branch
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git --no-pager log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE motis_git_ref
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git describe --always --tags --dirty=-dirty
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE motis_git_tag
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (
  "base/core/include/motis/core/version.h.in"
  "${CMAKE_BINARY_DIR}/generated/version.h"
)


################################
# Basic Compiler Flags
################################
foreach(flags CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

foreach(flags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
  string(REPLACE "-O3" "-O2" ${flags} "${${flags}}")
endforeach()

if (NOT MSVC)
  set(MOTIS_CXX_FLAGS "-std=c++1z -Wall -Wextra -DBOOST_NO_AUTO_PTR=1")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(MOTIS_CXX_FLAGS "${MOTIS_CXX_FLAGS} -Wno-maybe-uninitialized")
  endif()
  if (MOTIS_COV)
    set(MOTIS_CXX_FLAGS "${MOTIS_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif()
else()
  include(external_lib/address-typeahead/external_lib/googletest/googletest/cmake/internal_utils.cmake)
  set(MOTIS_CXX_FLAGS "-DBOOST_THREAD_USE_LIB -DWIN32_LEAN_AND_MEAN")
endif()

if(MINGW)
  add_definitions(-DWINVER=0x0501)
  add_definitions(-D_WIN32_WINNT=0x0501)
  add_definitions(-D_WIN32_IE=0x0501)
  add_definitions(-D_WEBSOCKETPP_NO_CPP11_THREAD_)
  add_definitions(-DBOOST_THREAD_USE_LIB)
  set(NETWORKING ws2_32 mswsock)
endif()


################################
# 3rd Party (Boost, pthread)
################################
if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()
find_package(Boost COMPONENTS system filesystem regex program_options thread context REQUIRED)
find_package(Threads)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})


################################
# Contained Dependencies
################################
include_directories(SYSTEM external_lib/address-typeahead/external_lib/expat/include)
include_directories(SYSTEM external_lib/address-typeahead/external_lib/leveldb/include)
include_directories(SYSTEM external_lib/address-typeahead/external_lib/googletest/googletest/include)
include_directories(SYSTEM external_lib/flatbuffers/include)
include_directories(SYSTEM external_lib/osmium/include)
include_directories(SYSTEM external_lib/miniz)
include_directories(SYSTEM external_lib/osrm-backend/include)
include_directories(SYSTEM external_lib/osrm-backend/third_party)
include_directories(SYSTEM external_lib/osrm-backend/external_lib/stxxl/include)
include_directories(SYSTEM ${CMAKE_BINARY_DIR}/external_lib/osrm-backend/external_lib/stxxl/include)
include_directories(SYSTEM external_lib/sparsehash/src)
include_directories(SYSTEM external_lib/websocketpp)
include_directories(external_lib/address-typeahead/include)
include_directories(external_lib/address-typeahead/external_lib/guess/include)
include_directories(external_lib/conf/include)
include_directories(external_lib/geo/include)
include_directories(external_lib/net/include)
include_directories(external_lib/parser/include)
include_directories(external_lib/ctx/include)
include_directories(external_lib/utl/include)

add_subdirectory(external_lib/address-typeahead EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/flatbuffers EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/osrm-backend EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/pugixml EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/conf EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/geo EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/net EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/parser EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/ctx EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/lmdb EXCLUDE_FROM_ALL)
add_subdirectory(external_lib/tar EXCLUDE_FROM_ALL)


################################
# MOTIS
################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base/bootstrap/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base/core/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base/loader/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base/module/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base/client/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test/include)

add_subdirectory(base/core EXCLUDE_FROM_ALL)
add_subdirectory(base/loader EXCLUDE_FROM_ALL)
add_subdirectory(base/module EXCLUDE_FROM_ALL)

file(GLOB all-modules RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/modules/*)
list(REMOVE_ITEM all-modules ".DS_Store")
string(REPLACE ";" " " modules-option "${all-modules}")
set(MODULES ${modules-option} CACHE STRING "Modules to load")

string(REPLACE " " ";" modules "${MODULES}")
list(REMOVE_ITEM modules
  "reliability"
  "path"
  "bikesharing"
)

set(module-targets "")
set(module-test-files "")
foreach(module ${all-modules})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/${module}/include)
endforeach(module)
foreach(module ${modules})
  add_subdirectory(modules/${module})
  list(APPEND module-targets "motis-${module}")
  list(APPEND module-test-files "modules/${module}/*_test.cc")
  list(APPEND module-itest-files "modules/${module}/*_itest.cc")
endforeach(module)

add_subdirectory(base/bootstrap EXCLUDE_FROM_ALL)
add_subdirectory(base/launcher)
add_subdirectory(base/client)


################################
# Tests
################################
configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_dir.h.in
  ${CMAKE_BINARY_DIR}/generated/test_dir.h
)
file(GLOB_RECURSE motis-test-files test/src/*.cc)
file(GLOB_RECURSE motis-modules-test-files ${module-test-files})
file(GLOB_RECURSE motis-base-test-files base/*_test.cc)
file(GLOB_RECURSE motis-loader-test-files base/loader/*_test.cc)
set_source_files_properties(${motis-loader-test-files} PROPERTIES COMPILE_DEFINITIONS FLATBUFFERS_64=1)
add_executable(motis-test EXCLUDE_FROM_ALL
  ${motis-test-files}
  ${motis-modules-test-files}
  ${motis-base-test-files}
)
set_target_properties(motis-test PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-test
  ${module-targets}
  motis-bootstrap
  motis-core
  motis-loader
  motis-module
  conf
  ${Boost_LIBRARIES}
)
target_link_libraries(motis-test gtest gtest_main)


################################
# Integration Tests
################################
configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_dir.h.in
  ${CMAKE_BINARY_DIR}/generated/test_dir.h
)
file(GLOB_RECURSE motis-modules-itest-files ${module-itest-files})
file(GLOB_RECURSE motis-base-itest-files base/*_itest.cc)
add_executable(motis-itest EXCLUDE_FROM_ALL
  ${motis-test-files}
  ${motis-modules-itest-files}
  ${motis-base-itest-files})
set_target_properties(motis-itest PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-itest
  ${module-targets}
  motis-bootstrap
  motis-core
  motis-loader
  motis-module
  conf
  ${Boost_LIBRARIES}
)
target_link_libraries(motis-itest gtest gtest_main)

################################
# Lint (clang-tidy)
################################
if(MOTIS_LINT)
  add_custom_target(lint
    /usr/bin/python
      ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-clang-tidy.py
        -exit-on-error
        -quiet
        -header-filter "^${CMAKE_CURRENT_SOURCE_DIR}/(base/)|(modules/)|(test/)"
        "^${CMAKE_CURRENT_SOURCE_DIR}/base/"
        "^${CMAKE_CURRENT_SOURCE_DIR}/modules/"
        "^${CMAKE_CURRENT_SOURCE_DIR}/test/"
    COMMENT "Running clang-tidy"
    VERBATIM
  )
  add_dependencies(lint motis motis-test motis-itest motis-eval motis-cli)
endif()


################################
# Lint (oclint)
################################
if(MOTIS_LINT)
  add_custom_target(oclint
    find
      ./modules/routing/ ./modules/guesser/ ./modules/osrm/ ./base/
      -type f
      -name "*.cc"
      ! -name "*test*"
      -print0
      |
      xargs
        -0
        oclint
          -enable-global-analysis
          -p ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running oclint"
    VERBATIM
  )
  add_dependencies(oclint motis motis-test motis-itest motis-eval motis-cli)
endif()


################################
# clang-format check
################################
add_custom_target(format-check
  find
      ${CMAKE_CURRENT_SOURCE_DIR}/base
      ${CMAKE_CURRENT_SOURCE_DIR}/modules
      ${CMAKE_CURRENT_SOURCE_DIR}/test
    -type f
    (
    -name "*.cc"
    -o
    -name "*.h"
    )
    -print0
  | xargs -0 clang-format -i
  COMMAND
  git status --porcelain | xargs -I {} -0 test -z \"{}\"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Checking source code formatting"
  VERBATIM
)


################################
# coverage report
################################
if (MOTIS_COV)
  set(LCOV_OPTS "")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(LCOV_OPTS "--gcov-tool=${CMAKE_CURRENT_SOURCE_DIR}/tools/llvm-gcov.sh")
  endif()

  add_custom_target(coverage
    # clean previous files
    rm -rf ./combined_coverage ./motis-test_coverage ./motis-itest_coverage
    COMMAND rm -f combined-test.info motis-test.info motis-itest.info
    COMMAND find . -name "*.gcda" -delete

    # combined (motis-test + motis-itest)
    COMMAND lcov --base-directory . --directory . --zerocounters -q
    COMMAND ./motis-test
    COMMAND ./motis-itest
    COMMAND lcov ${LCOV_OPTS} --base-directory . --directory . -c -o combined-test.info
    COMMAND lcov --remove combined-test.info "/usr*" -o combined-test.info
    COMMAND lcov --remove combined-test.info "/external_lib*" -o combined-test.info
    COMMAND lcov --remove combined-test.info "/test/*" -o combined-test.info
    COMMAND genhtml -o combined_coverage -t "motis-test + motis-itest" --num-spaces 4 combined-test.info

    # only motis-test
    COMMAND lcov --base-directory . --directory . --zerocounters -q
    COMMAND ./motis-test
    COMMAND lcov ${LCOV_OPTS} --base-directory . --directory . -c -o motis-test.info
    COMMAND lcov --remove motis-test.info "/usr*" -o motis-test.info
    COMMAND lcov --remove motis-test.info "/external_lib*" -o motis-test.info
    COMMAND lcov --remove motis-test.info "/test/*" -o motis-test.info
    COMMAND genhtml -o motis-test_coverage -t "motis-test" --num-spaces 4 motis-test.info

    # only motis-itest
    COMMAND lcov --base-directory . --directory . --zerocounters -q
    COMMAND ./motis-itest
    COMMAND lcov ${LCOV_OPTS} --base-directory . --directory . -c -o motis-itest.info
    COMMAND lcov --remove motis-itest.info "/usr*" -o motis-itest.info
    COMMAND lcov --remove motis-itest.info "/external_lib*" -o motis-itest.info
    COMMAND lcov --remove motis-itest.info "/test/*" -o motis-itest.info
    COMMAND genhtml -o motis-itest_coverage -t "motis-itest" --num-spaces 4 motis-itest.info
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating coverage report"
    VERBATIM
  )
  add_dependencies(coverage motis-test motis-itest)
endif()
