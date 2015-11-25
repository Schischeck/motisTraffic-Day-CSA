cmake_minimum_required(VERSION 2.8)
project(motis)

include_directories(include)

set(module-includes "\#include <string>")
set(module-includes "${module-includes}\n\#include <map>")
set(module-includes "${module-includes}\n\#include <memory>\n\n")
foreach(module ${modules})
  set(module-includes "${module-includes}#include \"motis/${module}/${module}.h\"\n")
endforeach(module)

set(module-map "std::vector<std::unique_ptr<motis::module::module>> build_modules() {")
set(module-map "${module-map}\n  std::vector<std::unique_ptr<motis::module::module>> m;")
foreach(module ${modules})
  set(module-map "${module-map}\n  m.emplace_back(new motis::${module}::${module}());")
endforeach(module)
set(module-map "${module-map}\n  return m;")
set(module-map "${module-map}\n}")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/modules.h.in
  ${CMAKE_BINARY_DIR}/generated/modules.h
)

file(GLOB_RECURSE motis-launcher-files src/*.cc)
add_executable(motis ${motis-launcher-files})

target_link_libraries(motis
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${NETWORKING}
  ${module-targets}
  conf
  motis-loader
  motis-module
)

set_target_properties(motis PROPERTIES COMPILE_DEFINITIONS _WEBSOCKETPP_CPP11_STL_)
set_target_properties(motis PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
