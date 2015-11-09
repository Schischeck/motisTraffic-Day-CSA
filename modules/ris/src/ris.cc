#include "motis/ris/ris.h"

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

#define MODE "ris.mode"
#define UPDATE_INTERVAL "ris.update_interval"
#define ZIP_FOLDER "ris.zip_folder"

#define MODE_DEFAULT "default"
#define MODE_FILESYSTEM "fileystem"
#define MODE_STATIC_DB "static-db"
#define MODE_SIMULATION "simulation"

using boost::system::error_code;
namespace fs = boost::filesystem;
using fs::directory_iterator;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::loader;

namespace motis {
namespace ris {

std::istream& operator>>(std::istream& in, motis::ris::mode& mode) {
  std::string token;
  in >> token;

  static const std::map<std::string, motis::ris::mode> map{
      {MODE_DEFAULT, motis::ris::mode::DEFAULT},
      {MODE_FILESYSTEM, motis::ris::mode::FILEYSTEM},
      {MODE_STATIC_DB, motis::ris::mode::STATIC_DB},
      {MODE_SIMULATION, motis::ris::mode::SIMULATION}};
  auto it = map.find(token);
  if (it == end(map)) {
    throw std::runtime_error("unknown mode of operation in ris module.");
  }

  mode = it->second;
  return in;
}

std::ostream& operator<<(std::ostream& out, motis::ris::mode const& mode) {
  switch (mode) {
    case motis::ris::mode::DEFAULT: out << MODE_DEFAULT; break;
    case motis::ris::mode::FILEYSTEM: out << MODE_FILESYSTEM; break;
    case motis::ris::mode::STATIC_DB: out << MODE_STATIC_DB; break;
    case motis::ris::mode::SIMULATION: out << MODE_SIMULATION; break;
    default: out << "unknown"; break;
  }
  return out;
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

ris::ris()
    : mode_(mode::DEFAULT),
      update_interval_(10),
      zip_folder_("ris"),
      simulation_time_(0) {}

po::options_description ris::desc() {
  po::options_description desc("RIS Module");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<mode>(&mode_)->default_value(mode_),
       "Mode of operation. Valid choices:\n"
       MODE_DEFAULT "= production style operation\n"
       MODE_FILESYSTEM "= no database\n"
       MODE_STATIC_DB "= no fileystem, no housekeeping\n"
       MODE_SIMULATION "= init db with fs, fwd via msg")
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
  out << "  " << MODE << ": " << mode_ << "\n"
      << "  " << UPDATE_INTERVAL << ": " << update_interval_ << "\n"
      << "  " << ZIP_FOLDER << ": " << zip_folder_;
}

void ris::init() {
  if (mode_ != mode::FILEYSTEM) {
    db_init();
    read_files_ = db_get_files();
  }

  if (mode_ == mode::SIMULATION) {
    init_simulation_db();
  } else {
    if (mode_ != mode::FILEYSTEM) {
      init_live_from_db();
    }

    if (mode_ != mode::STATIC_DB) {
      timer_ = make_unique<boost::asio::deadline_timer>(get_thread_pool());
      schedule_update(error_code());
    }
  }
}

void ris::init_simulation_db() {
  scoped_timer timer("RISML load simulation db");
  auto new_files = get_new_files();
  for (auto const& new_file : new_files) {
    db_put_messages(new_file, parse_xmls(read_zip_file(new_file)));
  }
}

void ris::on_msg(msg_ptr msg, sid, callback cb) {
  if (mode_ != mode::SIMULATION) {
    LOG(error) << "RIS received a message (but is not in simulation mode).";
    return cb({}, boost::system::error_code());
  }

  auto req = msg->content<RISForwardTimeRequest const*>();

  // TODO exception handling
  auto new_time = req->new_time();
  dispatch(pack(db_get_messages(simulation_time_, new_time)));
  simulation_time_ = new_time;
  LOG(info) << "RIS forwarded time to " << new_time;

  return cb({}, boost::system::error_code());
}

void ris::init_live_from_db() {
  auto sync = synced_sched<schedule_access::RO>();

  std::time_t now = std::time(nullptr);
  constexpr std::time_t kTwentyFourHours = 60 * 60 * 24;

  auto from = std::max(sync.sched().schedule_begin_, now - kTwentyFourHours);
  auto to = sync.sched().schedule_end_;

  dispatch(pack(db_get_messages(from, to)));
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

  // TODO implement housekeeping
  parse_zips();
}

void ris::parse_zips() {
  auto new_files = get_new_files();
  if (new_files.size() == 0) {
    return;
  }

  // the filenames must start with a lexicographically compareable timestamp
  std::sort(begin(new_files), end(new_files));

  scoped_timer timer("RISML parsing");
  LOG(info) << "parsing " << new_files.size() << " RISML ZIP files";
  for (auto const& new_file : new_files) {
    std::vector<ris_message> parsed_messages;
    try {
      parsed_messages = parse_xmls(read_zip_file(new_file));
    } catch (std::exception const& e) {
      LOG(error) << "bad zip file: " << e.what();
    }

    if (mode_ != mode::FILEYSTEM) {
      db_put_messages(new_file, parsed_messages);
    }
    dispatch(pack(parsed_messages));
  }
}

std::vector<std::string> ris::get_new_files() {
  fs::path path(zip_folder_);

  std::vector<std::string> new_files;
  if (fs::exists(path) && fs::is_directory(path)) {
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
  }

  return new_files;
}

}  // namespace ris
}  // namespace motis
