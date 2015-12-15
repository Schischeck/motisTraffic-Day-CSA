#include "motis/ris/ris.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "websocketpp/common/md5.hpp"

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
#define MAX_DAYS "ris.max_days"

#define MODE_LIVE "default"
#define MODE_SIMULATION "simulation"

using boost::system::error_code;
namespace fs = boost::filesystem;
using fs::directory_iterator;
using websocketpp::md5::md5_hash_hex;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::loader;

namespace motis {
namespace ris {

std::istream& operator>>(std::istream& in, motis::ris::mode& mode) {
  std::string token;
  in >> token;

  if (token == MODE_LIVE) {
    mode = motis::ris::mode::LIVE;
  } else if (token == MODE_SIMULATION) {
    mode = motis::ris::mode::SIMULATION;
  } else {
    throw std::runtime_error("unknown mode of operation in ris module.");
  }

  return in;
}

std::ostream& operator<<(std::ostream& out, motis::ris::mode const& mode) {
  switch (mode) {
    case motis::ris::mode::LIVE: out << MODE_LIVE; break;
    case motis::ris::mode::SIMULATION: out << MODE_SIMULATION; break;
    default: out << "unknown"; break;
  }
  return out;
}

template <typename T>
msg_ptr pack(std::vector<T> const& messages) {
  MessageCreator b;
  std::vector<Offset<MessageHolder>> message_offsets;
  for (auto& message : messages) {
    message_offsets.push_back(
        CreateMessageHolder(b, b.CreateVector(message.data(), message.size())));
  }

  b.CreateAndFinish(MsgContent_RISBatch,
                    CreateRISBatch(b, b.CreateVector(message_offsets)).Union());
  return make_msg(b);
}

ris::ris()
    : mode_(mode::LIVE),
      update_interval_(10),
      zip_folder_("ris"),
      max_days_(-1),
      simulation_time_(0) {}

po::options_description ris::desc() {
  po::options_description desc("RIS Module");
  // clang-format off
  desc.add_options()
      (MODE,
       po::value<mode>(&mode_)->default_value(mode_),
       "Mode of operation. Valid choices:\n"
       MODE_LIVE " = production style operation\n"
       MODE_SIMULATION " = init db with fs, fwd via msg")
      (UPDATE_INTERVAL,
       po::value<int>(&update_interval_)->default_value(update_interval_),
       "update interval in seconds")
      (ZIP_FOLDER,
       po::value<std::string>(&zip_folder_)->default_value(zip_folder_),
       "folder containing RISML ZIPs")
      (MAX_DAYS,
       po::value<int>(&max_days_)->default_value(max_days_),
       "periodically delete messages older than n days (-1 = infinite)");
  // clang-format on
  return desc;
}

void ris::print(std::ostream& out) const {
  out << "  " << MODE << ": " << mode_ << "\n"
      << "  " << UPDATE_INTERVAL << ": " << update_interval_ << "\n"
      << "  " << ZIP_FOLDER << ": " << zip_folder_ << "\n"
      << "  " << MAX_DAYS << ": " << max_days_;
}

std::time_t days_ago(long days) {
  std::time_t now = std::time(nullptr);
  constexpr std::time_t kTwentyFourHours = 60 * 60 * 24;
  return now - kTwentyFourHours * days;
}

void ris::init() {
  db_init();
  read_files_ = db_get_files();

  fill_database();

  if (mode_ == mode::LIVE) {
    auto sync = synced_sched<schedule_access::RO>();

    auto from = std::max(sync.sched().schedule_begin_, days_ago(1));
    auto to = sync.sched().schedule_end_;

    dispatch(pack(db_get_messages(from, to)));

    timer_ = make_unique<boost::asio::deadline_timer>(get_thread_pool());
    schedule_update(error_code());
  }
}

void ris::fill_database() {
  scoped_timer timer("RISML load database");
  auto new_files = get_new_files();
  for (auto const& new_file : new_files) {
    db_put_messages(new_file, parse_xmls(read_zip_file(new_file)));
  }
}

void ris::on_msg(msg_ptr msg, sid, callback cb) {
  switch (msg->content_type()) {
    case MsgContent_RISForwardTimeRequest: return handle_forward_time(msg, cb);
    case MsgContent_HTTPRequest: return handle_zipfile_upload(msg, cb);
    default: assert(false);
  }
}

void ris::handle_forward_time(msg_ptr msg, callback cb) {
  if (mode_ != mode::SIMULATION) {
    LOG(error) << "RIS received a fwd time msg (but is not in sim mode).";
    return cb({}, boost::system::error_code());
  }

  auto req = msg->content<RISForwardTimeRequest const*>();
  auto new_time = req->new_time();
  dispatch(pack(db_get_messages(simulation_time_, new_time)));
  simulation_time_ = new_time;
  LOG(info) << "RIS forwarded time to " << new_time;

  return cb({}, boost::system::error_code());
}

void ris::handle_zipfile_upload(msg_ptr msg, callback cb) {
  auto req = msg->content<HTTPRequest const*>();
  try {
    auto buf = parser::buffer(req->content()->c_str(), req->content()->size());
    auto parsed_messages = parse_xmls(read_zip_buf(buf));
    db_put_messages(
        md5_hash_hex(reinterpret_cast<const unsigned char*>(buf.data()),
                     buf.size()),
        parsed_messages);
    dispatch(pack(parsed_messages));
  } catch (std::exception const& e) {
    LOG(error) << "bad zip file: " << e.what();
    MessageCreator b;
    b.CreateAndFinish(
        MsgContent_HTTPResponse,
        CreateHTTPResponse(b, HTTPStatus_INTERNAL_SERVER_ERROR,
                           b.CreateVector(std::vector<Offset<HTTPHeader>>()),
                           b.CreateString(e.what()))
            .Union());
    return cb(make_msg(b), boost::system::error_code());
  }

  return cb({}, boost::system::error_code());
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
  // TODO db_clean_messages(days_ago(max_days_));
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

    db_put_messages(new_file, parsed_messages);
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
