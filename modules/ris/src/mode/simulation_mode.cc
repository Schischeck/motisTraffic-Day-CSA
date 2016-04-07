// #include "motis/ris/mode/simulation_mode.h"

// #include "motis/core/access/time_access.h"
// #include "motis/core/common/logging.h"
// #include "motis/ris/detail/pack_msgs.h"
// #include "motis/ris/database.h"
// #include "motis/ris/error.h"
// #include "motis/ris/ris.h"

// using boost::system::system_error;
// using namespace motis::logging;
// using namespace motis::module;
// using namespace motis::ris::detail;

// namespace motis {
// namespace ris {
// namespace mode {

// std::string to_string(std::time_t const time) {
//   char buf[sizeof "2011-10-08t07:07:09z-0430"];
//   strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&time));
//   return buf;
// }

// void simulation_mode::init_async() {
//   base_mode::init_async();

//   // TODO verify simulation logic (times etc.)
//   // TODO apply this logic to the live mode
//   LOG(info) << "RIS starting in SIMULATION mode.";
//   auto lock = module_->synced_sched2();
//   auto from = external_schedule_begin(lock.sched());
//   auto to = external_schedule_end(lock.sched());

//   LOG(debug) << "using interval " << to_string(from) << " - " << to_string(to)
//              << " (" << from << " - " << to << ")";

//   auto start_time = db_get_forward_start_time(from, to);
//   if (start_time == kDBInvalidTimestamp) {
//     return;
//   }
//   forward_time(from, to, start_time, module_->sim_init_time_,
//                [this](msg_ptr, boost::system::error_code) {
//                  LOG(info) << "RIS forwarding done, SIMULATION time is now: "
//                            << to_string(simulation_time_);
//                });
// }

// void simulation_mode::on_msg(msg_ptr msg, sid, callback cb) {
//   if (msg->content_type() != MsgContent_RISForwardTimeRequest) {
//     throw system_error(error::unexpected_message);
//   }

//   auto lock = module_->synced_sched2();
//   auto from = external_schedule_begin(lock.sched());
//   auto to = external_schedule_end(lock.sched());

//   auto req = msg->content<RISForwardTimeRequest const*>();
//   return forward_time(from, to, simulation_time_, req->new_time(), cb);
// }



// }  // namespace mode
// }  // namespace ris
// }  // namespace motis
