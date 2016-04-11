#include "motis/ris/mode/live_mode.h"

#include <ctime>
#include <memory>

#include "websocketpp/common/md5.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/raii.h"
#include "motis/loader/util.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/database.h"
#include "motis/ris/error.h"
#include "motis/ris/ris.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

using boost::asio::deadline_timer;
using boost::system::system_error;
using boost::system::error_code;
using websocketpp::md5::md5_hash_hex;
using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;
using namespace motis::ris::risml;
using namespace std::placeholders;

namespace motis {
namespace ris {
namespace mode {

void live_mode::init_async() {
  base_mode::init_async();

  forward();

  timer_ = std::make_unique<deadline_timer>(module_->get_thread_pool2());
  schedule_update(error_code());

  r.register_op("/ris/upload", std::bind(&live_mode::handle_upload, this, _1);
}

void live_mode::schedule_update(error_code e) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  MOTIS_FINALLY([this]() {
    timer_->expires_from_now(boost::posix_time::seconds(10));
    timer_->async_wait(std::bind(&live_mode::schedule_update, this, _1));
  });

  parse_zips();
  db_clean_messages(days_ago(module_->max_days_));
}

void live_mode::on_msg(msg_ptr const& msg) {
  auto req = motis_content(HTTPRequest, msg);

  try {
    auto buf = parser::buffer(req->content()->c_str(), req->content()->size());
    auto parsed_messages = parse_xmls(read_zip_buf(buf));
    db_put_messages(
        md5_hash_hex(reinterpret_cast<const unsigned char*>(buf.data()),
                     buf.size()),
        parsed_messages);
    
    ctx::await_all(motis_publish(pack_msgs(parsed_messages)));

  } catch (std::exception const& e) {
    LOG(logging::error) << "bad zip file: " << e.what();
    throw boost::system::system_error(error::bad_message);
  }
}

void live_mode::parse_zips() {
  auto new_files = find_new_files(conf_->input_folder_, ".zip", read_files_);
  if (new_files.size() == 0) {
    return;
  }

  // the filenames must start with a lexicographically compareable timestamp
  std::sort(begin(new_files), end(new_files));

  // scoped_timer timer("RISML parsing");
  // LOG(info) << "parsing " << new_files.size() << " RISML ZIP files";
  std::vector<future> futures;
  for (auto const& new_file : new_files) {
    std::vector<ris_message> msgs;
    try {
      msgs = parse_xmls(read_zip_file(new_file));
    } catch (std::exception const& e) {
      LOG(logging::error) << "bad zip file: " << e.what();
    }

    // always mark the file as read to prevent re-read in the next update cycle
    read_files_.insert(new_file);
    db_put_messages(new_file, msgs);

    if(msgs.size() > 0) {
      ctx::await_all(futures);
      futures = motis_publish(pack_msgs(msgs));
    }
  }

  system_time_changed();
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
