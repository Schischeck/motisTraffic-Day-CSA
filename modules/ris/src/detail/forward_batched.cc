#include "motis/ris/detail/forward_batched.h"

namespace motis {
namespace ris {
namespace detail {

std::string to_string(std::time_t const time) {
  char buf[sizeof "2011-10-08t07:07:09z-0430"];
  strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&time));
  return buf;
}

void forward_batched(std::time_t const schedule_begin,
                     std::time_t const schedule_end,
                     std::time_t const start_time,  //
                     std::time_t const end_time, // 
                     db_ptr const& database) {


}

}  // namespace detail
}  // namespace ris
}  // namespace motis


// struct forwarder {
//   void forward(std::shared_ptr<forwarder> self, boost::system::error_code ec) {
//     while (true) {
//       if (ec) {
//         *simulation_time = curr_time;
//         LOG(info) << "RIS forwarding failed " << ec.message();
//         return finished_cb({}, ec);
//       }

//       curr_time = next_time;
//       next_time = std::min(curr_time + 3600, end_time);
//       if (next_time == curr_time) {
//         *simulation_time = curr_time;
//         LOG(info) << "RIS forwarded time to " << to_string(curr_time);
//         return finished_cb({}, error::ok);
//       }

//       LOG(info) << "RIS forwarding time to " << to_string(next_time);

//       manual_timer t{"database io"};
//       auto msgs = db_get_messages(from, to, curr_time, next_time);
//       t.stop_and_print();

//       if (msgs.size() == 0) {
//         continue;
//       }

//       namespace p = std::placeholders;
//       return module->dispatch2(
//           pack_msgs(msgs), 0,
//           std::bind(&forwarder::forward, this, self, p::_2));
//     }
//   };

//   std::time_t from, to;  // schedule bounds
//   std::time_t end_time, curr_time, next_time;

//   ris* module;
//   std::time_t* simulation_time;

//   callback finished_cb;
// };

// void simulation_mode::forward_time(std::time_t const from, std::time_t const to,
//                                    std::time_t const start_time,
//                                    std::time_t const end_time,
//                                    callback finished_cb) {
//   auto fwd = std::make_shared<forwarder>();
//   fwd->from = from;
//   fwd->to = to;
//   fwd->end_time = end_time;
//   fwd->curr_time = start_time;
//   fwd->next_time = start_time;
//   fwd->finished_cb = finished_cb;
//   fwd->module = module_;
//   fwd->simulation_time = &simulation_time_;

//   fwd->forward(fwd, error::ok);
// }