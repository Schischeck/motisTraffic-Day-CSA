#pragma once

#include <vector>

#include "parser/buffer.h"

#include "motis/ris/database.h"
#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {

inline std::vector<parser::buffer> pack(char const* str) {
  std::vector<parser::buffer> vec;
  parser::buffer buf(str);
  vec.emplace_back(std::move(buf));
  return vec;
}

struct ris_database_util {
  ris_database_util& add_entry(std::time_t const earliest,
                               std::time_t const latest,
                               std::time_t const timestamp,
                               std::string const& str = "no_payload") {
    entries_.emplace_back(earliest, latest, timestamp, str);
    return *this;
  }

  ris_database_util& finish_packet(db_ptr const& db,
                                   std::string const& fname = "no_file") {
    db_put_messages(db, fname, entries_);
    entries_.clear();
    return *this;
  }

  std::vector<ris_message> entries_;
};

inline char const* blob_to_cstr(std::basic_string<uint8_t> const& buf) {
  return reinterpret_cast<char const*>(buf.data());
}

}  // namespace ris
}  // namespace motis
