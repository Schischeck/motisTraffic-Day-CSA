project(motis)
cmake_minimum_required(VERSION 3.0)

set(module-includes "\#include <string>")
set(module-includes "${module-includes}\n\#include <map>")
set(module-includes "${module-includes}\n\#include <memory>\n\n")
foreach(module ${modules})
  set(module-includes "${module-includes}#include \"motis/${module}/${module}.h\"\n")
endforeach(module)

set(module-map "inline std::vector<std::unique_ptr<motis::module::module>> build_modules() {")
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

file(GLOB_RECURSE motis-bootstrap-files src/*.cc)
add_library(motis-bootstrap STATIC ${motis-bootstrap-files})
set_target_properties(motis-bootstrap PROPERTIES COMPILE_FLAGS ${MOTIS_CXX_FLAGS})
target_link_libraries(motis-bootstrap motis-loader motis-module ${module-targets})
