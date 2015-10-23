#include "motis/ris/ris.h"

#include <boost/algorithm/string/predicate.hpp>
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "motis/ris/zip_reader.h"
#include "motis/ris/risml_parser.h"
#include "motis/core/common/logging.h"

#define UPDATE_INTERVAL "ris.update_interval"
#define ZIP_FOLDER "ris.zip_folder"

using boost::system::error_code;
namespace fs = boost::filesystem;
using fs::directory_iterator;

namespace motis {
namespace ris {

ris::ris() : update_interval_(10), zip_folder_("ris") {}

po::options_description ris::desc() {
  po::options_description desc("RIS Module");
  // clang-format off
  desc.add_options()
      (UPDATE_INTERVAL,
       po::value<int>(&update_interval_)->default_value(update_interval_),
       "update interval in seconds")
      (ZIP_FOLDER,
       po::value<std::string>(&zip_folder_)->default_value(zip_folder_),
       "folder containing RISML ZIPs");
  // clang-format on
  return desc;
}

void ris::print(std::ostream& out) const {
  out << "  " << UPDATE_INTERVAL << ": " << update_interval_ << "\n"
      << "  " << ZIP_FOLDER << ": " << zip_folder_;
}

void ris::init() {
  timer_ = std::unique_ptr<boost::asio::deadline_timer>(
      new boost::asio::deadline_timer(get_thread_pool()));

  schedule_update(error_code());
}

#include <iostream>

void ris::parse_zips() {
  logging::scoped_timer t("ris :: parse_zips");

  auto new_files = get_new_files();
  std::cout << "parsing " << new_files.size() << " new files." << std::endl;

  int i = 0;
  for(auto const& new_file : new_files) {
    auto msg = parse_xmls(read_zip_file(new_file));

    std::cout << i++ << std::endl;
  }
}

std::vector<std::string> ris::get_new_files() {
  fs::path path(zip_folder_);

  if(!(fs::exists(path) && fs::is_directory(path))) {
    throw std::runtime_error(ZIP_FOLDER " is not a directory");
  }

  std::vector<std::string> new_files;
  for (auto it = directory_iterator(path); it != directory_iterator(); ++it) {
    if (!fs::is_regular_file(it->status())) {
      continue;
    }
    
    auto filename = it->path().string();
    if(!boost::algorithm::iends_with(filename, ".zip")) {
      continue;
    }

    if(read_files_.insert(filename).second) {
      new_files.push_back(filename);
    }
  }

  return new_files;
}

void ris::schedule_update(error_code e) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  parse_zips();

  timer_->expires_from_now(boost::posix_time::seconds(10));
  timer_->async_wait(
      std::bind(&ris::schedule_update, this, std::placeholders::_1));
}

}  // namespace ris
}  // namespace motis
