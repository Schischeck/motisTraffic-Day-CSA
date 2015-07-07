file(GLOB absolute-dir-path ${dir})

file(WRITE ${header} "")
file(GLOB_RECURSE bins ${dir}/*)

set(symbols "const unsigned char* symbols[] = {")
set(filenames "const char* filenames[] = {")

set(number-of-symbols 0)

foreach(bin ${bins})
  # Generate variable name from file name:
  # Replace "." and "/" by "_".
  get_filename_component(filename ${bin} NAME)
  string(REGEX REPLACE "\\.| " "_" filename ${filename})
  string(REGEX REPLACE "/" "_" filename ${filename})

  # Convert file content to hex.
  file(READ ${bin} filedata HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})

  # Write hexadecimal file contents.
  file(APPEND ${header} "static const unsigned char ${filename}[] = {${filedata}0x00};\n")

  # Remember relative file path.
  file(RELATIVE_PATH path ${absolute-dir-path} ${bin})
  file(APPEND ${header} "static const char ${filename}_filename[] = \"${path}\";\n")

  # Build symbol and filename table.
  set(symbols "${symbols}${filename},")
  set(filenames "${filenames}${filename}_filename,")

  # Increment schema count.
  math(EXPR number-of-symbols "${number-of-symbols}+1")
endforeach()

set(symbols ${symbols} "}\;\n")
set(filenames ${filenames} "}\;\n")

file(APPEND ${header} ${symbols})
file(APPEND ${header} ${filenames})

file(APPEND ${header} "static const unsigned int number_of_symbols = ${number-of-symbols};\n")