#include "motis/ris/ris.h"

#include <iostream>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/raii.h"
#include "motis/loader/util.h"
#include "motis/ris/database.h"
#include "motis/ris/risml_parser.h"
#include "motis/ris/ris_message.h"
#include "motis/ris/zip_reader.h"

#define UPDATE_INTERVAL "ris.update_interval"
#define ZIP_FOLDER "ris.zip_folder"

using boost::system::error_code;
namespace fs = boost::filesystem;
using fs::directory_iterator;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::loader;

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

template <typename T>
msg_ptr pack(std::vector<T> const& messages) {
  FlatBufferBuilder fbb;
  std::vector<Offset<MessageHolder>> message_offsets;
  for (auto& message : messages) {
    message_offsets.push_back(CreateMessageHolder(
        fbb, fbb.CreateVector(message.data(), message.size())));
  }

  fbb.Finish(CreateMessage(
      fbb, MsgContent_RISBatch,
      CreateRISBatch(fbb, fbb.CreateVector(message_offsets)).Union()));
  return make_msg(fbb);
}

void ris::init() {
  timer_ = make_unique<boost::asio::deadline_timer>(get_thread_pool());

  db_init();
  read_files_ = db_get_files();
  dispatch(pack(db_get_messages(0, 0)));  // TODO

  schedule_update(error_code());
}

void ris::parse_zips() {
  auto new_files = get_new_files();
  if (new_files.size() == 0) {
    return;
  }

  scoped_timer timer("RISML parsing");
  LOG(info) << "parsing " << new_files.size() << " RISML ZIP files";
  for (auto const& new_file : new_files) {
    std::vector<ris_message> parsed_messages;
    try {
      parsed_messages = parse_xmls(read_zip_file(new_file));
    } catch (std::exception const& e) {
      LOG(error) << "bad zip file: " << e.what();
    }
    db_put_messages(new_file, parsed_messages);
    dispatch(pack(parsed_messages));
  }
}

std::vector<std::string> ris::get_new_files() {
  fs::path path(zip_folder_);

  if (!(fs::exists(path) && fs::is_directory(path))) {
    throw std::runtime_error(ZIP_FOLDER " is not a directory");
  }

  std::vector<std::string> new_files;
  for (auto it = directory_iterator(path); it != directory_iterator(); ++it) {
    if (!fs::is_regular_file(it->status())) {
      continue;
    }

    auto filename = it->path().string();
    if (!boost::algorithm::iends_with(filename, ".zip")) {
      continue;
    }

    if (read_files_.insert(filename).second) {
      new_files.push_back(filename);
    }
  }

  return new_files;
}

void ris::schedule_update(error_code e) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  MOTIS_FINALLY([this]() {
    timer_->expires_from_now(boost::posix_time::seconds(10));
    timer_->async_wait(
        std::bind(&ris::schedule_update, this, std::placeholders::_1));
  });

  parse_zips();
}

}  // namespace ris
}  // namespace motis
