#include "motis/routes/routes.h"

#include "motis/module/context/get_schedule.h"
#include "motis/routes/routes_section.h"

#include "boost/filesystem.hpp"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace routes {

routes::routes() : module("Routes", "routes"), file_loaded_(false) {
  string_param(routes_file_, "rail_roads.raw", "railroads_folder",
               "folder containing railroad data");
};
routes::~routes() = default;

void routes::init(registry& r) {
  if (boost::filesystem::exists(routes_file_)) {
     routes_buf_ = parser::file(routes_file_.c_str(), "r").content();
     file_loaded_ = true;
   }
  r.register_op("/routes/section",
                [this](msg_ptr const& m) { return routes_section(m); });
}

msg_ptr routes::routes_section(msg_ptr const& m) {
  message_creator b;
  auto msg = motis_content(RoutesSectionReq, m);
  auto const& sched = get_schedule();
  auto const& departure = get_station(sched, msg->departure()->str());
  auto const& arrival = get_station(sched, msg->arrival()->str());
  auto clasz = msg->clasz();

  b.create_and_finish(
      MsgContent_RoutesSectionRes,
      (file_loaded_ ? motis::routes::routes_railroad_sec_with_data(
                          departure, arrival, clasz, b, routes_buf_)
                    : motis::routes::routes_railroad_sec_without_data(
                          departure, arrival, clasz, b))
          .Union());
  return make_msg(b);
}

}  // namespace routes
}  // namespace motis
