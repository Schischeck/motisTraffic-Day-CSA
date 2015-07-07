include_directories(${CMAKE_SOURCE_DIR}/external_lib/flatbuffers/include)

set(protocol-headers-dir ${CMAKE_BINARY_DIR}/generated/motis/protocol)
file(GLOB_RECURSE protocol-files ${CMAKE_SOURCE_DIR}/protocol/*.fbs)

set(resource-header ${protocol-headers-dir}/resources.h)
add_custom_command(
  COMMAND
    ${CMAKE_COMMAND}
      -Dheader:string="${protocol-headers-dir}/resources.h"
      -Ddir:string="${CMAKE_SOURCE_DIR}/protocol"
      -P "${CMAKE_SOURCE_DIR}/cmake/GenerateProtocolResource.cmake"
  DEPENDS ${protocol-files}
  OUTPUT ${resource-header}
)

foreach(file ${protocol-files})
  get_filename_component(filename ${file} NAME)
  string(REGEX REPLACE "\\.fbs$" "_generated.h" generated-header "${filename}")
  set(generated-header "${protocol-headers-dir}/${generated-header}")

  add_custom_command(
    OUTPUT ${generated-header}
    COMMAND flatc -c -o "${protocol-headers-dir}" "${file}"
    DEPENDS flatc ${file}
  )

  list(APPEND generated-headers ${generated-header})
endforeach(file)
