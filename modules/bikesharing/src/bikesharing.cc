#include "motis/bikesharing/bikesharing.h"

#include <iostream>
#include <numeric>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "parser/file.h"

#include "motis/bikesharing/nextbike_parser.h"
#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/protocol/Message_generated.h"

#define NEXTBIKE_PATH "bikesharing.nextbike_path"

using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
namespace fs = boost::filesystem;
namespace po = boost::program_options;
using fs::directory_iterator;

namespace motis {
namespace bikesharing {

bikesharing::bikesharing() : nextbike_path_("") {}

po::options_description bikesharing::desc() {
  po::options_description desc("bikesharing Module");
  // clang-format off
  desc.add_options()
      (NEXTBIKE_PATH,
       po::value<std::string>(&nextbike_path_)->default_value(nextbike_path_),
       "Where nextbike snapshots can be found (may be folder or single file)");
  // clang-format on
  return desc;
}

void bikesharing::print(std::ostream& out) const {
  out << "  " << NEXTBIKE_PATH << ": " << nextbike_path_;
}

std::vector<std::string> get_nextbike_files(fs::path const& base);

void bikesharing::init() {
  fs::path path(nextbike_path_);
  if (!fs::exists(path)) {
    return;
  }

  auto files = get_nextbike_files(path);
  LOG(info) << "loading " << files.size() << " NEXTBIKE XML files";
  
  manual_timer parse_timer("NEXTBIKE parsing");
  snapshot_merger merger;
  for (auto const& filename : files) {
    auto xml_string = parser::file(filename.c_str(), "r").content();
    auto timestamp = nextbike_filename_to_timestamp(filename);
    auto snapshot = nextbike_parse_xml(std::move(xml_string));
    merger.add_snapshot(timestamp, snapshot);
  }
  parse_timer.stop_and_print();

  manual_timer merge_timer("NEXTBIKE merging");
  auto merged = merger.merged();
  merge_timer.stop_and_print();
}

std::vector<std::string> get_nextbike_files(fs::path const& base) {
  std::vector<std::string> files;
  if (fs::is_regular_file(base)) {
    files.push_back(base.string());
  } else if (fs::is_directory(base)) {
    for (auto it = directory_iterator(base); it != directory_iterator(); ++it) {
      if (!fs::is_regular_file(it->status())) {
        continue;
      }

      auto filename = it->path().string();
      if (boost::algorithm::iends_with(filename, ".xml")) {
        files.push_back(filename);
      }
    }
  }
  return files;
}

void bikesharing::on_msg(msg_ptr, sid, callback cb) {
  return cb({}, boost::system::error_code());
}

}  // namespace bikesharing
}  // namespace motis
