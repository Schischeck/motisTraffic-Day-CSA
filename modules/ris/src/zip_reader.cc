#include "motis/ris/zip_reader.h"

#include "miniz.c"

#include "parser/file.h"

#include "motis/core/common/raii.h"

using namespace parser;

namespace motis {
namespace ris {

std::vector<buffer> read_zip_buf(buffer const& b) {
  std::vector<buffer> file_contents;

  mz_zip_archive zip_archive{};
  memset(&zip_archive, 0, sizeof(zip_archive));

  auto archive =
      make_raii(&zip_archive, [&](mz_zip_archive* a) { mz_zip_reader_end(a); });
  if (!mz_zip_reader_init_mem(archive, b.buf_, b.size_, 0)) {
    throw std::runtime_error("invalid zip archive");
  }

  int num_files = mz_zip_reader_get_num_files(&zip_archive);
  for (int i = 0; i < num_files; ++i) {
    size_t size;
    auto extracted =
        make_raii(mz_zip_reader_extract_to_heap(&zip_archive, i, &size, 0),
                  [](void* p) { mz_free(p); });
    file_contents.emplace_back(reinterpret_cast<char const*>(extracted.get()),
                               size);
  }

  return file_contents;
}

std::vector<buffer> read_zip_file(std::string const& filename) {
  return read_zip_buf(file(filename.c_str(), "r").content());
}

}  // namespace ris
}  // namespace motis
