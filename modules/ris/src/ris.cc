#include "motis/ris/ris.h"

#include "boost/program_options.hpp"

#define UPDATE_INTERVAL "ris.update_interval"
#define ZIP_FOLDER "ris.zip_folder"

using boost::system::error_code;

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

void ris::parse_zips() {}

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
