#include "motis/ris/mode/live_mode.h"

#include <ctime>
#include <memory>

#include "websocketpp/common/md5.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/raii.h"
#include "motis/module/context/get_io_service.h"
#include "motis/module/context/motis_wrap.h"
#include "motis/loader/util.h"
#include "motis/ris/database.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/max_timestamp.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/error.h"
#include "motis/ris/ris.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

using boost::asio::deadline_timer;
using boost::system::system_error;
using boost::system::error_code;
using websocketpp::md5::md5_hash_hex;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;
using namespace motis::ris::risml;
using namespace std::placeholders;

namespace motis {
namespace ris {
namespace mode {

void live_mode::init(motis::module::registry& r) {
  base_mode::init(r);
  r.register_op("/ris/upload", std::bind(&live_mode::handle_upload, this, _1));
}

void live_mode::init_async() {
  base_mode::init_async();

  forward(std::time(nullptr));  // forward to current time

  timer_ = std::make_unique<deadline_timer>(get_io_service());
  schedule_update(error_code());
}

void live_mode::schedule_update(error_code e) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  MOTIS_FINALLY([this]() {
    timer_->expires_from_now(boost::posix_time::seconds(10));
    timer_->async_wait(motis_wrap(std::function<void(error_code)>(
        [this](error_code e) { schedule_update(e); })));
  });

  parse_zips();
}

void live_mode::handle_upload(msg_ptr const& msg) {
  auto req = motis_content(HTTPRequest, msg);
  try {
    system_time_forward(std::time(nullptr), [&] {
      auto buf =
          parser::buffer(req->content()->c_str(), req->content()->size());
      auto parsed_messages = parse_xmls(read_zip_buf(buf));
      auto hash = md5_hash_hex(
          reinterpret_cast<unsigned char const*>(buf.data()), buf.size());
      db_put_messages(db_, hash, parsed_messages);

      if (parsed_messages.size() > 0) {
        ctx::await_all(motis_publish(pack_msgs(parsed_messages)));
        return max_timestamp(parsed_messages);
      }
      return 0l;
    });
  } catch (std::exception const& e) {
    LOG(logging::error) << "bad zip file: " << e.what();
    throw std::system_error(error::bad_zip_file);
  }
}

void live_mode::parse_zips() {
  system_time_forward(std::time(nullptr), [&] {
    auto new_files = find_new_files(conf_->input_folder_, ".zip", read_files_);
    // the filenames must start with a lexicographically compareable
    std::sort(begin(new_files), end(new_files));
    LOG(info) << "parsing " << new_files.size() << " RISML ZIP files";

    std::time_t timestamp = 0;
    std::vector<future> futures;
    for (auto const& new_file : new_files) {
      std::vector<ris_message> msgs;
      try {
        scoped_timer timer("RISML parsing");
        msgs = parse_xmls(read_zip_file(new_file));
      } catch (std::exception const& e) {
        LOG(logging::error) << "bad zip file: " << e.what();
      }

      // always mark the file as read to prevent re-read in the next update
      read_files_.insert(new_file);
      db_put_messages(db_, new_file, msgs);

      if (msgs.size() > 0) {
        timestamp = std::max(timestamp, max_timestamp(msgs));
        ctx::await_all(futures);
        futures = motis_publish(pack_msgs(msgs));
      }
    }
    ctx::await_all(futures);
    return timestamp;
  });
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
