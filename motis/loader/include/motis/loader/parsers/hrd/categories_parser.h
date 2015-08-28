#pragma once

#include <cinttypes>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "boost/filesystem/path.hpp"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct categories_parser {

  // fbs_category := (category_info, is_export_candidate) TODO (string, offset)
  typedef std::pair<flatbuffers::Offset<flatbuffers::String>, bool>
      fbs_category_t;

  typedef flatbuffers::Offset<flatbuffers::Vector<
      flatbuffers::Offset<flatbuffers::String>>> fbs_categories_t;

  void parse(boost::filesystem::path const& categories,
             boost::filesystem::path const& providers);

  void parse(loaded_file const& categories, loaded_file const& providers);

  // NOTE: Implicitly adds the corresponding category information to the export
  // data.
  flatbuffers::Offset<flatbuffers::String> get_category_info(
      uint32_t category_key);

  fbs_categories_t get_export_data(flatbuffers::FlatBufferBuilder const&);

  std::map<uint32_t, fbs_category_t> fbs_categories_;
};

}  // hrd
}  // loader
}  // motis
