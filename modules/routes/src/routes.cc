#include "motis/routes/routes.h"

#include "motis/module/context/get_schedule.h"
#include "motis/routes/routes_section.h"

#include "boost/filesystem.hpp"

#include "motis/protocol/RoutesAllSectionRes_generated.h"

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
  r.register_op("/routes/all",
                [this](msg_ptr const& m) { return all_sections(m); });
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

msg_ptr routes::all_sections(msg_ptr const& m) {
  message_creator b;
  auto msg = motis_content(RoutesAllSectionReq, m);
  auto railroad_sections = GetRoutesSections(routes_buf_.buf_);
  std::vector<flatbuffers::Offset<RoutesSection>> sections;
  for (auto const& section : *railroad_sections->sections()) {
    std::vector<double> rail_section;
    for (auto const d : *section->section()) {
      rail_section.push_back(d);
    }
    if (section->clasz() == msg->clasz()) {
      sections.push_back(CreateRoutesSection(b, section->from(), section->to(),
                                             section->clasz(),
                                             b.CreateVector(rail_section)));
    }
  }
  b.create_and_finish(
      MsgContent_RoutesAllSectionRes,
      CreateRoutesAllSectionRes(b, b.CreateVector(sections)).Union());
  return make_msg(b);
}

}  // namespace routes
}  // namespace motis
