#include "motis/ris/mode/live_mode.h"

#include <ctime>

#include "websocketpp/common/md5.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/raii.h"
#include "motis/loader/util.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/database.h"
#include "motis/ris/error.h"
#include "motis/ris/ris.h"
#include "motis/ris/risml_parser.h"
#include "motis/ris/zip_reader.h"

using boost::system::system_error;
using boost::system::error_code;
using websocketpp::md5::md5_hash_hex;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;

namespace motis {
namespace ris {
namespace mode {

std::time_t days_ago(long days) {
  std::time_t now = std::time(nullptr);
  constexpr std::time_t kTwentyFourHours = 60 * 60 * 24;
  return now - kTwentyFourHours * days;
}

void live_mode::init_async() {
  base_mode::init_async();

  auto sync = module_->synced_sched2();

  auto from = std::max(sync.sched().schedule_begin_, days_ago(1));
  auto to = sync.sched().schedule_end_;

  module_->dispatch2(pack_msgs(db_get_messages(from, to)));

  timer_ =
      make_unique<boost::asio::deadline_timer>(module_->get_thread_pool2());
  schedule_update(error_code());
}

void live_mode::schedule_update(error_code e) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  MOTIS_FINALLY([this]() {
    namespace p = std::placeholders;
    timer_->expires_from_now(boost::posix_time::seconds(10));
    timer_->async_wait(std::bind(&live_mode::schedule_update, this, p::_1));
  });

  parse_zips();
  db_clean_messages(days_ago(module_->max_days_));
}

void live_mode::on_msg(msg_ptr msg, sid, callback cb) {
  if (msg->content_type() != MsgContent_HTTPRequest) {
    return cb({}, error::unexpected_message);
  }

  auto req = msg->content<HTTPRequest const*>();
  try {
    auto buf = parser::buffer(req->content()->c_str(), req->content()->size());
    auto parsed_messages = parse_xmls(read_zip_buf(buf));
    db_put_messages(
        md5_hash_hex(reinterpret_cast<const unsigned char*>(buf.data()),
                     buf.size()),
        parsed_messages);
    module_->dispatch2(pack_msgs(parsed_messages));
  } catch (std::exception const& e) {
    LOG(logging::error) << "bad zip file: " << e.what();
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

void live_mode::parse_zips() {
  auto new_files = find_new_files(module_->zip_folder_, &read_files_);
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
      LOG(logging::error) << "bad zip file: " << e.what();
    }

    // always mark the file as read to prevent re-read in the next update cycle
    read_files_.insert(new_file);
    db_put_messages(new_file, parsed_messages);

    module_->dispatch2(pack_msgs(parsed_messages));
  }
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
