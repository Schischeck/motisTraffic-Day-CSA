#include "motis/ris/zip_reader.h"

#include "zip.h"

#include "parser/file.h"

using namespace parser;

namespace motis {
namespace ris {

struct lzip_error {
  lzip_error() { zip_error_init(&error_); }
  ~lzip_error() { zip_error_fini(&error_); }
  operator zip_error_t*() { return &error_; }
  zip_error_t error_;
};

template <typename T, typename DestructFun>
struct raii {
  raii(T&& el, DestructFun&& destruct)
      : el_(std::forward<T>(el)),
        destruct_(std::forward<DestructFun>(destruct)),
        omit_destruct_(false) {}

  ~raii() {
    if (!omit_destruct_) {
      destruct_(el_);
    }
  }

  T& get() { return el_; }
  operator T() { return el_; }

  T el_;
  DestructFun destruct_;
  bool omit_destruct_;
};

template <typename T, typename DestructFun>
raii<T, DestructFun> make_raii(T&& el, DestructFun&& destruct) {
  return {std::forward<T>(el), std::forward<DestructFun>(destruct)};
};

std::vector<buffer> read_zip_buf(buffer const& b) {
  std::vector<buffer> file_contents;

  lzip_error err;

  auto src = make_raii(zip_source_buffer_create(b.data(), b.size(), 0, err),
                       [](zip_source_t* src) { zip_source_free(src); });
  if (src.get() == nullptr) {
    throw std::runtime_error("zip error: open create");
  }

  auto zip =
      make_raii(zip_open_from_source(src, ZIP_RDONLY, err), [&src](zip_t* zip) {
        zip_close(zip);
        src.omit_destruct_ = true;
      });
  if (zip.get() == nullptr) {
    throw std::runtime_error("zip error: open");
  }

  // Iterate files contained in the ZIP Archive.
  zip_int64_t num_entries = zip_get_num_entries(zip, 0);
  for (zip_int64_t i = 0; i < num_entries; ++i) {
    zip_stat_t sb;
    if (zip_stat_index(zip, i, 0, &sb) != 0) {
      throw std::runtime_error("zip error: stat_index");
    }

    auto zip_file = make_raii(zip_fopen_index(zip, i, 0),
                              [](zip_file_t* f) { zip_fclose(f); });
    if (zip_file.get() == nullptr) {
      throw std::runtime_error("zip error: zip_fopen");
    }

    size_t bytes_read = 0;
    buffer content(sb.size);
    while (bytes_read != sb.size) {
      auto len = zip_fread(zip_file, content.data() + bytes_read, 4096);
      if (len < 0) {
        throw std::runtime_error("zip error: read");
      }
      bytes_read += len;
    }

    file_contents.emplace_back(std::move(content));
  }

  return file_contents;
}

std::vector<buffer> read_zip_file(std::string const& filename) {
  return read_zip_buf(file(filename.c_str(), "r").content());
}

}  // namespace ris
}  // namespace motis
