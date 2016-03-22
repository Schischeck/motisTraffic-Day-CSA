#include "motis/ris/mode/simulation_mode.h"

#include "motis/core/access/time_access.h"
#include "motis/core/common/logging.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/database.h"
#include "motis/ris/error.h"
#include "motis/ris/ris.h"

using boost::system::system_error;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;

namespace motis {
namespace ris {
namespace mode {

std::string to_string(std::time_t const time) {
  char buf[sizeof "2011-10-08t07:07:09z-0430"];
  strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&time));
  return buf;
}

void simulation_mode::init_async() {
  base_mode::init_async();

  // TODO verify simulation logic (times etc.)
  // TODO apply this logic to the live mode
  LOG(info) << "RIS starting in SIMULATION mode.";
  auto lock = module_->synced_sched2();
  auto from = external_schedule_begin(lock.sched());
  auto to = external_schedule_end(lock.sched());

  auto start_time = db_get_forward_start_time(from, to);
  if (start_time == kDBInvalidTimestamp) {
    return;
  }
  forward_time(from, to, start_time, module_->sim_init_time_,
               [this](msg_ptr, boost::system::error_code) {
                 LOG(info) << "RIS forwarding done, SIMULATION time is now: "
                           << to_string(simulation_time_);
               });
}

void simulation_mode::on_msg(msg_ptr msg, sid, callback cb) {
  if (msg->content_type() != MsgContent_RISForwardTimeRequest) {
    throw system_error(error::unexpected_message);
  }

  auto lock = module_->synced_sched2();
  auto from = external_schedule_begin(lock.sched());
  auto to = external_schedule_end(lock.sched());

  auto req = msg->content<RISForwardTimeRequest const*>();
  return forward_time(from, to, simulation_time_, req->new_time(), cb);
}

struct forwarder {
  void forward(std::shared_ptr<forwarder> self, boost::system::error_code ec) {
    while (true) {
      if (ec) {
        *simulation_time = curr_time;
        LOG(info) << "RIS forwarding failed " << ec.message();
        return finished_cb({}, ec);
      }

      curr_time = next_time;
      next_time = std::min(curr_time + 3600, end_time);
      if (next_time == curr_time) {
        *simulation_time = curr_time;
        LOG(info) << "RIS forwarded time to " << to_string(curr_time);
        return finished_cb({}, error::ok);
      }

      LOG(info) << "RIS forwarding time to " << to_string(next_time);

      manual_timer t{"database io"};
      auto msgs = db_get_messages(from, to, curr_time, next_time);
      t.stop_and_print();

      if (msgs.size() == 0) {
        continue;
      }

      namespace p = std::placeholders;
      return module->dispatch2(
          pack_msgs(msgs), 0,
          std::bind(&forwarder::forward, this, self, p::_2));
    }
  };

  std::time_t from, to;  // schedule bounds
  std::time_t end_time, curr_time, next_time;

  ris* module;
  std::time_t* simulation_time;

  callback finished_cb;
};

void simulation_mode::forward_time(std::time_t const from, std::time_t const to,
                                   std::time_t const start_time,
                                   std::time_t const end_time,
                                   callback finished_cb) {
  auto fwd = std::make_shared<forwarder>();
  fwd->from = from;
  fwd->to = to;
  fwd->end_time = end_time;
  fwd->curr_time = start_time;
  fwd->next_time = start_time;
  fwd->finished_cb = finished_cb;
  fwd->module = module_;
  fwd->simulation_time = &simulation_time_;

  fwd->forward(fwd, error::ok);
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
