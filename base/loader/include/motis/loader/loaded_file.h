#pragma once

#include "boost/filesystem/path.hpp"

#include "parser/buffer.h"
#include "parser/cstr.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {

struct loaded_file {
  loaded_file() = default;

  loaded_file(char const* filename, char const* str)
      : name_(filename), buf_(str) {}

  loaded_file(char const* filename, parser::buffer&& buf)
      : name_(filename), buf_(std::move(buf)) {}

  explicit loaded_file(boost::filesystem::path p)
      : name_(p.filename().string()), buf_(load_file(p)) {}

  loaded_file(loaded_file const&) = delete;

  loaded_file(loaded_file&& o) {
    name_ = std::move(o.name_);
    buf_ = std::move(o.buf_);
  }

  loaded_file& operator=(loaded_file const&) = delete;

  loaded_file& operator=(loaded_file&& o) {
    name_ = std::move(o.name_);
    buf_ = std::move(o.buf_);
    return *this;
  }

  char const* name() const { return name_.c_str(); }
  parser::cstr content() const { return {buf_.data(), buf_.size()}; }

private:
  std::string name_;
  parser::buffer buf_;
};

}  // namespace loader
}  // namespace motis
