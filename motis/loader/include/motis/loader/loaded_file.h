#pragma once

#include "boost/filesystem/path.hpp"

#include "parser/cstr.h"
#include "parser/buffer.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {

struct loaded_file {
  loaded_file() = default;
  loaded_file(char const* filename, char const* str)
      : name_(filename), buf_(str), content_(buf_.data(), buf_.size()) {}
  loaded_file(char const* filename, parser::buffer&& buf)
      : name_(filename),
        buf_(std::move(buf)),
        content_(buf_.data(), buf_.size()) {}
  explicit loaded_file(boost::filesystem::path p)
      : name_(p.filename().string().c_str()),
        buf_(load_file(p)),
        content_(buf_.data(), buf_.size()) {}

  loaded_file(loaded_file const&) = delete;
  loaded_file(loaded_file&& o) {
    buf_ = std::move(o.buf_);
    name_ = std::move(o.name_);
    content_ = o.content_;
    o.content_ = {};
  }

  loaded_file& operator=(loaded_file const&) = delete;
  loaded_file& operator=(loaded_file&& o) {
    buf_ = std::move(o.buf_);
    name_ = std::move(o.name_);
    content_ = o.content_;
    o.content_ = {};
    return *this;
  }

  const char* name_;
  parser::buffer buf_;
  parser::cstr content_;
};

}  // namespace loader
}  // namespace motis
